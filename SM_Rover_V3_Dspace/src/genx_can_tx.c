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
 
 * Created on: 19-08-2026
 *     Author: SasiPrasanthSakhinal
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

#include "smartwheels_rover.h"
#include "genx_common.h"


struct smartwheels_rover_rover_acc_status_t can_tx_msg_rover_acc_status_instance;


/***********************************************
 * ANCIT_CG_TxMsg_Stub_Start
 ***********************************************/
 
/**
 * Message ID : 0x301
 * DLC : 5;
**/
void ancit_can_setup_tx_message_ROVER_ACC_STATUS(uint8_t idx) {
	can_tx_messages[idx].dataInfo.data_length = 5;
	can_tx_msg_rover_acc_status_instance.real_dmin=smartwheels_rover_rover_acc_status_real_dmin_encode(ggenx.Dmin);
	can_tx_msg_rover_acc_status_instance.real_vset=smartwheels_rover_rover_acc_status_real_vset_encode(ggenx.Vset);
	can_tx_msg_rover_acc_status_instance.real_obj=smartwheels_rover_rover_acc_status_real_obj_encode(ggenx.Ultra_Distance);
	can_tx_msg_rover_acc_status_instance.pwm=smartwheels_rover_rover_acc_status_pwm_encode(ggenx.PWM);

	smartwheels_rover_rover_acc_status_pack(can_tx_messages[idx].tx_data,&can_tx_msg_rover_acc_status_instance,5);
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
//CAN_TX_MSG_IDX_ROVER_ACC_STATUS// 
{.idx = CAN_TX_MSG_IDX_ROVER_ACC_STATUS,// 
.instance_id =0,// 
.msg_id_type = FLEXCAN_MSG_ID_STD,// 
.msg_id = 0x301,// 
.enabled = true, // 
.msg_type = CAN_TX_MSG_TYPE_CYCLIC,// 
.set_interval_ms = 10.0,// 
.setupMessage = ancit_can_setup_tx_message_ROVER_ACC_STATUS// 
}

};
/***********************************************
 * ANCIT_CG_TxMsg_Register_End
 ***********************************************/
 
void genx_can_tx_init(void) {
	memset(can_tx_messages, 0, sizeof(can_tx_messages));

	gVars.can_tx_max = CAN_TX_MSG_MAX;

	ancit_can_tx_init();
	ancit_can_tx_onstart();
	/* NOTE: CAN_LOOPBACK_MODE register setup is done in main.c AFTER
	 * ancit_can_start(), because ancit_can_start() performs a FlexCAN
	 * SOFTRST via ancit_driver_can_init() which clears CTRL1[LPB].
	 * Setting LPB here would be wiped before it takes effect. */
}

#endif //CAN_TX_CONFIGURED
