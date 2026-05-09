/*
 * genx_can_tx.c
 *
 * Copyright (c) 2024-2025 ANCIT Consulting Pvt Ltd
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.

 * Created on: 01-07-2025
 *     Author: Narayan
 *
 */

/***********************************************
 * ANCIT_CG_TxMsg_Stub_Start
 ***********************************************/
#include "genx_can_tx.h"
#include "ancit_can_tx.h"
#include <stdlib.h>
#include <string.h>

#ifdef CAN_TX_CONFIGURED
tx_state_t sm_state;
can_tx_message_t can_tx_messages[CAN_TX_MSG_MAX];

#include "smart_wheels_hl.h"
#include "ancit_head_light.h"

struct smart_wheels_hl_head_light_left_btn_ctrl_t can_tx_msg_head_light_left_btn_ctrl_instance;
struct smart_wheels_hl_head_light_right_btn_ctrl_t can_tx_msg_head_light_right_btn_ctrl_instance;

extern int count;
extern uint8_t button_status;

/***********************************************
 * ANCIT_CG_TxMsg_Stub_Start
 ***********************************************/

/**
 * Message ID : 0x321
 * DLC : 1;
 **/
void ancit_can_setup_tx_message_head_light_left_btn_ctrl(uint8_t idx)
{
	can_tx_messages[idx].dataInfo.data_length = 1;

	can_tx_msg_head_light_left_btn_ctrl_instance.parking_light = finalLightStatus_left.parking_light;
	can_tx_msg_head_light_left_btn_ctrl_instance.indicator_light = finalLightStatus_left.indicator_light;
	can_tx_msg_head_light_left_btn_ctrl_instance.head_light_level = finalLightStatus_left.head_light_level;
	can_tx_msg_head_light_left_btn_ctrl_instance.head_light_beam = finalLightStatus_left.head_light_beam;

	smart_wheels_hl_head_light_left_btn_ctrl_pack(can_tx_messages[idx].tx_data, &can_tx_msg_head_light_left_btn_ctrl_instance, 1);
}
/**
 * Message ID : 0x322
 * DLC : 1;
 **/
void ancit_can_setup_tx_message_head_light_right_btn_ctrl(uint8_t idx)
{
	can_tx_messages[idx].dataInfo.data_length = 1;

	can_tx_msg_head_light_right_btn_ctrl_instance.parking_light = finalLightStatus_right.parking_light;
	can_tx_msg_head_light_right_btn_ctrl_instance.indicator_light = finalLightStatus_right.indicator_light;
	can_tx_msg_head_light_right_btn_ctrl_instance.head_light_level = finalLightStatus_right.head_light_level;
	can_tx_msg_head_light_right_btn_ctrl_instance.head_light_beam = finalLightStatus_right.head_light_beam;

	smart_wheels_hl_head_light_right_btn_ctrl_pack(can_tx_messages[idx].tx_data, &can_tx_msg_head_light_right_btn_ctrl_instance, 1);
}

void ancit_can_setup_tx_message_display(uint8_t idx){
	can_tx_messages[idx].dataInfo.data_length = 8;

	if(TUNNEL_DURATION - count >= 0){
		can_tx_messages[idx].tx_data[0]= TUNNEL_DURATION + 1 - count;
			can_tx_messages[idx].tx_data[1]= TUNNEL_DURATION+1;

	}
	else{
			can_tx_messages[idx].tx_data[0]= TUNNEL_DURATION+1;
			can_tx_messages[idx].tx_data[1]= TUNNEL_DURATION+1;
	}

}

/***********************************************
 * ANCIT_CG_TxMsg_Stub_End
 ***********************************************/

/* Tx message registration defaults
 *  - Message Index
 *  - Standard message ID
 *  - Enabled
 *  - Message Type (OnStart, Cyclic, OnEvent etc.,
 *  - Callback function for data and dlc setup before Tx
 */
/***********************************************
 * ANCIT_CG_TxMsg_Register_Start
 ***********************************************/
can_tx_message_registration_params_t tx_reg[CAN_TX_MSG_MAX] = {
	// CAN_TX_MSG_IDX_head_light_left_btn_ctrl//
	{
		.idx = CAN_TX_MSG_IDX_head_light_left_btn_ctrl,						//
		.msg_id = 0x321,													//
		.enabled = true,													//
		.msg_type = CAN_TX_MSG_TYPE_CYCLIC,									//
		.set_interval_ms = 100.0,											//
		.setupMessage = ancit_can_setup_tx_message_head_light_left_btn_ctrl //
	},
	// CAN_TX_MSG_IDX_head_light_right_btn_ctrl//
	{
		.idx = CAN_TX_MSG_IDX_head_light_right_btn_ctrl,					 //
		.msg_id = 0x322,													 //
		.enabled = true,													 //
		.msg_type = CAN_TX_MSG_TYPE_CYCLIC,									 //
		.set_interval_ms = 100.0,											 //
		.setupMessage = ancit_can_setup_tx_message_head_light_right_btn_ctrl //
	},

	{
		.idx = CAN_TX_MSG_IDX_display,					 //
		.msg_id = 0x325,													 //
#ifdef AUTO_DIP_FEATURE_ENABLED
		.enabled = true,													 //
#else
		.enabled = false,
#endif
		.msg_type = CAN_TX_MSG_TYPE_CYCLIC,									 //
		.set_interval_ms = 100.0,											 //
		.setupMessage = ancit_can_setup_tx_message_display //
	}

};
/***********************************************
 * ANCIT_CG_TxMsg_Register_End
 ***********************************************/

void genx_can_tx_init(void)
{
	memset(can_tx_messages, 0, sizeof(can_tx_messages));

	gVars.can_tx_max = CAN_TX_MSG_MAX;

	ancit_can_tx_init();
	ancit_can_tx_onstart();
}

#endif // CAN_TX_CONFIGURED
