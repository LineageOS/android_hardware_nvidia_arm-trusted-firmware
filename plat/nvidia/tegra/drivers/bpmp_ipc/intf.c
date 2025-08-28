/*
 * Copyright (c) 2017-2023, NVIDIA CORPORATION. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <bpmp_ipc.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <errno.h>
#include <lib/mmio.h>
#include <lib/utils_def.h>
#include <stdbool.h>
#include <string.h>
#include <tegra_def.h>
#include <tegra_private.h>
#include <tegra_platform.h>

#include "intf.h"
#include "ivc.h"

/**
 * Holds IVC channel data
 */
struct ccplex_bpmp_channel_data {
	/* Buffer for incoming data */
	struct frame_data *ib;

	/* Buffer for outgoing data */
	struct frame_data *ob;
};

#ifdef PLATFORM_SOCKET_COUNT
static struct ccplex_bpmp_channel_data s_channel[PLATFORM_SOCKET_COUNT];
static struct ivc ivc_ccplex_bpmp_channel[PLATFORM_SOCKET_COUNT];
#else
static struct ccplex_bpmp_channel_data s_channel[1U];
static struct ivc ivc_ccplex_bpmp_channel[1U];
#endif

static uint32_t num_sockets = 1U;

static inline uint32_t get_my_socket_id(void)
{
	return MPIDR_AFFLVL3_VAL(read_mpidr());
}

/*
 * Helper functions to access the HSP doorbell registers
 */
static inline uint32_t hsp_db_read(uint32_t socket, uint32_t reg)
{
	return mmio_read_32(TEGRA_HSP_DBELL_BASE + reg +
			TEGRA_REMOTE_CHIP_MEM_OFFSET(socket));

}

static inline void hsp_db_write(uint32_t socket, uint32_t reg, uint32_t val)
{
	mmio_write_32((TEGRA_HSP_DBELL_BASE + reg +
			TEGRA_REMOTE_CHIP_MEM_OFFSET(socket)), val);
}

/*******************************************************************************
 *      IVC wrappers for CCPLEX <-> BPMP communication.
 ******************************************************************************/

static void tegra_bpmp_ring_bpmp_doorbell(uint32_t socket);

/*
 * Get the next frame where data can be written.
 */
static struct frame_data *tegra_bpmp_get_next_out_frame(uint32_t socket)
{
	struct frame_data *frame;
	const struct ivc *ch = &ivc_ccplex_bpmp_channel[socket];

	frame = (struct frame_data *)tegra_ivc_write_get_next_frame(ch);
	if (frame == NULL) {
		ERROR("%s: Error in getting next frame, exiting\n", __func__);
	} else {
		s_channel[socket].ob = frame;
	}

	return frame;
}

static void tegra_bpmp_signal_slave(uint32_t socket)
{
	(void)tegra_ivc_write_advance(&ivc_ccplex_bpmp_channel[socket]);
	tegra_bpmp_ring_bpmp_doorbell(socket);
}

static int32_t tegra_bpmp_free_master(uint32_t socket)
{
	return tegra_ivc_read_advance(&ivc_ccplex_bpmp_channel[socket]);
}

static bool tegra_bpmp_slave_acked(uint32_t socket)
{
	struct frame_data *frame;
	bool ret = true;

	frame = (struct frame_data *)tegra_ivc_read_get_next_frame(&ivc_ccplex_bpmp_channel[socket]);
	if (frame == NULL) {
		ret = false;
	} else {
		s_channel[socket].ib = frame;
	}

	return ret;
}

static struct frame_data *tegra_bpmp_get_cur_in_frame(uint32_t socket)
{
	return s_channel[socket].ib;
}

/*
 * Enables BPMP to ring CCPlex doorbell
 *
 * When initializing IVC, we enable the doorbell so that BPMP can ring it.
 * however, we should enable doorbell on the socket where ATF is running.
 * Since the initialization is done on boot socket, which is socket 0, only
 * enable it for socket 0.
 */
static void tegra_bpmp_enable_ccplex_doorbell(void)
{
	uint32_t reg;
	uint32_t socket = get_my_socket_id();

	reg = hsp_db_read(socket, HSP_DBELL_1_ENABLE);
	reg |= HSP_MASTER_BPMP_BIT;
	hsp_db_write(socket, HSP_DBELL_1_ENABLE, reg);
}

/*
 * CCPlex rings the BPMP doorbell
 */
static void tegra_bpmp_ring_bpmp_doorbell(uint32_t socket)
{
	/*
	 * Any writes to this register has the same effect,
	 * uses master ID of the write transaction and set
	 * corresponding flag.
	 */
	hsp_db_write(socket, HSP_DBELL_3_TRIGGER, HSP_MASTER_CCPLEX_BIT);
}

/*
 * Returns true if CCPLex can ring BPMP doorbell, otherwise false.
 * This also signals that BPMP is up and ready.
 */
static bool tegra_bpmp_can_ccplex_ring_doorbell(uint32_t socket)
{
	uint32_t reg;

	/* check if ccplex can communicate with bpmp */
	reg = hsp_db_read(socket, HSP_DBELL_3_ENABLE);

	return ((reg & HSP_MASTER_CCPLEX_BIT) != 0U);
}

static int32_t tegra_bpmp_wait_for_slave_ack(uint32_t socket)
{
	uint32_t timeout = TIMEOUT_RESPONSE_FROM_BPMP_US;

	while (!tegra_bpmp_slave_acked(socket) && (timeout != 0U)) {
		udelay(1);
		timeout--;
	};

	return ((timeout == 0U) ? -ETIMEDOUT : 0);
}

/*
 * Notification from the ivc layer
 */
static void tegra_bpmp_ivc_notify(const struct ivc *ivc)
{
	(void)(ivc);

	tegra_bpmp_ring_bpmp_doorbell(ivc->socket);
}

/*
 * This function does IPC with BPMP-FW, but does not wait for response.
 */
static void tegra_bpmp_ipc_send_req_no_resp(uint32_t mrq, void *p_out,
				uint32_t size_out, uint32_t socket)
{
	struct frame_data *frame = tegra_bpmp_get_next_out_frame(socket);
	void *p_fdata;

	if ((p_out == NULL) || (size_out > IVC_DATA_SZ_BYTES) ||
	    (frame == NULL)) {
		ERROR("%s: invalid parameters, exiting\n", __func__);
		return;
	}

	/* prepare the command frame */
	frame->mrq = mrq;
	frame->flags = FLAG_DO_ACK;
	p_fdata = frame->data;
	(void)memcpy(p_fdata, p_out, (size_t)size_out);
	dsbsy();
	assert(memcmp(p_fdata, p_out, (size_t)size_out) == 0);

	/* signal the slave */
	tegra_bpmp_signal_slave(socket);
}

/*
 * Atomic send/receive API, which means it waits until slave acks
 */
static int32_t tegra_bpmp_ipc_send_req_atomic(uint32_t mrq, void *p_out,
			uint32_t size_out, void *p_in, uint32_t size_in,
			uint32_t socket)
{
	struct frame_data *frame = tegra_bpmp_get_next_out_frame(socket);
	struct frame_data *f_in = NULL;
	int32_t ret = 0;
	void *p_fdata;

	if ((p_out == NULL) || (size_out > IVC_DATA_SZ_BYTES) ||
	    (frame == NULL)) {
		ERROR("%s: invalid parameters, exiting\n", __func__);
		return -EINVAL;
	}

	/* prepare the command frame */
	frame->mrq = mrq;
	frame->flags = FLAG_DO_ACK;
	p_fdata = frame->data;
	(void)memcpy(p_fdata, p_out, (size_t)size_out);
	dsbsy();
	assert(memcmp(p_fdata, p_out, (size_t)size_out) == 0);

	/* signal the slave */
	tegra_bpmp_signal_slave(socket);

	/* wait for slave to ack */
	ret = tegra_bpmp_wait_for_slave_ack(socket);
	if (ret < 0) {
		ERROR("%s: wait for slave failed (%d)\n", __func__, ret);
		return ret;
	}

	/* retrieve the response frame */
	if ((size_in <= IVC_DATA_SZ_BYTES) && (p_in != NULL)) {

		f_in = tegra_bpmp_get_cur_in_frame(socket);
		if (f_in == NULL) {
			ERROR("Failed to get next input frame!\n");
		} else {
			p_fdata = f_in->data;
			(void)memcpy(p_in, p_fdata, (size_t)size_in);
		}
	}

	ret = tegra_bpmp_free_master(socket);
	if (ret < 0) {
		ERROR("%s: free master failed (%d)\n", __func__, ret);
	}

	return ret;
}

/*
 * Initializes the BPMP<--->CCPlex communication path.
 */
static int32_t tegra_bpmp_ipc_init_one(uint32_t socket)
{
	size_t msg_size;
	uint32_t frame_size, timeout;
	int32_t error = 0;
	struct bpmp_ipc_platform_data *bpmp_ipc;

	/* wait for BPMP to actually ring the doorbell */
	timeout = TIMEOUT_RESPONSE_FROM_BPMP_US;
	while ((timeout != 0U) && !tegra_bpmp_can_ccplex_ring_doorbell(socket)) {
		udelay(1); /* bpmp turn-around time */
		timeout--;
	}

	if (timeout == 0U) {
		ERROR("%s: BPMP firmware is not ready\n", __func__);
		return -ENOTSUP;
	}

	INFO("%s: BPMP handshake completed\n", __func__);

	/* get bpmp_ipc data */
	bpmp_ipc = plat_get_bpmp_ipc_data();
	if (bpmp_ipc == NULL) {
		ERROR("%s: bpmp_ipc interface is not supported\n", __func__);
		return -ENOTSUP;
	}

	msg_size = tegra_ivc_align(IVC_CMD_SZ_BYTES);
	frame_size = (uint32_t)tegra_ivc_total_queue_size(msg_size);
	if (frame_size > bpmp_ipc->bpmp_ipc_map_size) {
		ERROR("%s: carveout size is not sufficient\n", __func__);
		return -EINVAL;
	}

	error = tegra_ivc_init(&ivc_ccplex_bpmp_channel[socket],
				bpmp_ipc[socket].bpmp_ipc_rx_base,
				bpmp_ipc[socket].bpmp_ipc_tx_base,
				1U, frame_size, tegra_bpmp_ivc_notify);
	if (error != 0) {

		ERROR("%s: IVC init failed (%d)\n", __func__, error);

	} else {

		/* reset channel */
		tegra_ivc_channel_reset(&ivc_ccplex_bpmp_channel[socket]);

		/* wait for notification from BPMP */
		while (tegra_ivc_channel_notified(&ivc_ccplex_bpmp_channel[socket]) != 0) {
			/*
			 * Interrupt BPMP with doorbell each time after
			 * tegra_ivc_channel_notified() returns non zero
			 * value.
			 */
			tegra_bpmp_ring_bpmp_doorbell(socket);
		}

		INFO("%s: All communication channels initialized\n", __func__);
	}

	return error;
}

int32_t tegra_bpmp_ipc_init(uint32_t socket_count)
{
	int32_t ret = 0;
	uint32_t socket;

	num_sockets = socket_count;

	/* allow bpmp to ring CCPLEX's doorbell */
	tegra_bpmp_enable_ccplex_doorbell();

	for (socket = 0; (socket < num_sockets) && (ret == 0); socket++) {
		ivc_ccplex_bpmp_channel[socket].socket = socket;
		ret = tegra_bpmp_ipc_init_one(socket);
	}
	return ret;
}

/* Handler to reset a hardware module */
int32_t tegra_bpmp_ipc_reset_module(uint32_t rst_id)
{
	int32_t ret;
	struct mrq_reset_request req = {
		.cmd = (uint32_t)CMD_RESET_MODULE,
		.reset_id = rst_id
	};

	/* only GPCDMA/XUSB_PADCTL resets are supported */
	assert((rst_id == TEGRA_RESET_ID_XUSB_PADCTL) ||
	       (rst_id == TEGRA_RESET_ID_GPCDMA));

	ret = tegra_bpmp_ipc_send_req_atomic(MRQ_RESET, &req,
			(uint32_t)sizeof(req), NULL, 0, 0U);
	if (ret != 0) {
		ERROR("%s: failed for module %d with error %d\n", __func__,
		      rst_id, ret);
	}

	return ret;
}

/* Handler to enable clock for a given ID */
int tegra_bpmp_ipc_enable_clock(uint32_t clk_id)
{
	int ret;
	struct mrq_clk_request req;
	struct cmd_clk_is_en_resp resp = { 0 };

	/* only SE clocks are supported */
	if (clk_id != TEGRA_CLK_SE) {
		return -ENOTSUP;
	}

	/* prepare the MRQ_CLK command */
	req.cmd_and_id = make_mrq_clk_cmd(CMD_CLK_ENABLE, clk_id);

	ret = tegra_bpmp_ipc_send_req_atomic(MRQ_CLK, &req, (uint32_t)sizeof(req),
			NULL, 0, 0U);
	if (ret != 0) {
		ERROR("%s: failed for module %d with error %d\n", __func__,
		      clk_id, ret);
	}

	/* prepare the MRQ_CLK command to check if clk is actually enabled */
	req.cmd_and_id = make_mrq_clk_cmd(CMD_CLK_IS_ENABLED, clk_id);

	ret = tegra_bpmp_ipc_send_req_atomic(MRQ_CLK, &req, (uint32_t)sizeof(req),
			&resp, (uint32_t)sizeof(resp), 0U);
	if (resp.state == 0) {
		ERROR("%s: clock enable failed for module %d with error %d\n", __func__,
		      clk_id, ret);
	}

	return ret;
}

/* Handler to disable clock for a given ID */
int tegra_bpmp_ipc_disable_clock(uint32_t clk_id)
{
	int ret;
	struct mrq_clk_request req;

	/* only SE clocks are supported */
	if (clk_id != TEGRA_CLK_SE) {
		return -ENOTSUP;
	}

	/* prepare the MRQ_CLK command */
	req.cmd_and_id = make_mrq_clk_cmd(CMD_CLK_DISABLE, clk_id);

	ret = tegra_bpmp_ipc_send_req_atomic(MRQ_CLK, &req, (uint32_t)sizeof(req),
			NULL, 0, 0U);
	if (ret != 0) {
		ERROR("%s: failed for module %d with error %d\n", __func__,
		      clk_id, ret);
	}

	return ret;
}

void tegra_bpmp_ipc_system_shutdown(uint32_t state)
{
	uint32_t socket, my_socket;
	struct mrq_shutdown_request req;

	assert((state == TEGRA_SYSTEM_OFF) || (state == TEGRA_SYSTEM_RESET));

	req.state = state;

	my_socket = get_my_socket_id();
	for (socket = 0U; socket < num_sockets; socket++) {
		if (socket == my_socket) {
			continue;
		}

		tegra_bpmp_ipc_send_req_no_resp(MRQ_SHUTDOWN,
				&req, (uint32_t)sizeof(req), socket);
	}

	tegra_bpmp_ipc_send_req_no_resp(MRQ_SHUTDOWN,
				&req, (uint32_t)sizeof(req), my_socket);
}

/* Handler to retrieve thresholds for NDIV offsets */
int tegra_bpmp_ipc_get_ndiv_offsets(uint32_t socket, struct mrq_gears_response *resp)
{
	int ret;
	uint32_t req = 0U;

	ret = tegra_bpmp_ipc_send_req_atomic(MRQ_GEARS, &req, (uint32_t)sizeof(req),
			resp, (uint32_t)sizeof(struct mrq_gears_response), socket);
	if (ret != 0) {
		ERROR("%s: MRQ_GEARS failed (error: %d)\n", __func__, ret);
	}

	return ret;
}
