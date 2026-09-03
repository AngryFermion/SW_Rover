/*
 * genx_can_rx.c
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
#include <string.h>
#include "genx_can_rx.h"

#ifdef CAN_RX_CONFIGURED
g_struct_can_rx_t can_rx[CAN_RX_MSG_MAX];

#include "smartwheels_rover.h"
#include "genx_common.h"


struct smartwheels_rover_rover_vehicle_cmd_t can_rx_msg_rover_vehicle_cmd_instance;
struct smartwheels_rover_rover_acc_param_t can_rx_msg_rover_acc_param_instance;




/***********************************************
 * ANCIT_CG_Rx_Notfn_Start
 ***********************************************/
 
void ancit_can_rx_notification_ROVER_VEHICLE_CMD(void) {
//Received data will be available in can_rx[CAN_RX_MSG_IDX_ROVER_VEHICLE_CMD].recvMsg 
#ifdef CAN_IDS_CONFIGURED
uint32_t actual_id = can_rx[CAN_RX_MSG_IDX_ROVER_VEHICLE_CMD].recvMsg.msgId;
genx_ids_check_message(actual_id, can_rx[CAN_RX_MSG_IDX_ROVER_VEHICLE_CMD].recvMsg.dataLen, CAN_RX_MSG_IDX_ROVER_VEHICLE_CMD);
if (actual_id != 0x201) { return; } /* Not our message - skip unpack */
#endif
smartwheels_rover_rover_vehicle_cmd_unpack(&can_rx_msg_rover_vehicle_cmd_instance,can_rx[CAN_RX_MSG_IDX_ROVER_VEHICLE_CMD].recvMsg.data,can_rx[CAN_RX_MSG_IDX_ROVER_VEHICLE_CMD].recvMsg.dataLen);

	ggenx.VACC =can_rx_msg_rover_vehicle_cmd_instance.vacc_enable;
	ggenx.ACC =can_rx_msg_rover_vehicle_cmd_instance.acc_enable;
	ggenx.Speed =can_rx_msg_rover_vehicle_cmd_instance.speed_cmd;
	ggenx.Mode =can_rx_msg_rover_vehicle_cmd_instance.direction_cmd;
#ifdef ANCIT_CAN_GATEWAY_ENABLED
ancit_can_gateway_process(0x201, can_rx[CAN_RX_MSG_IDX_ROVER_VEHICLE_CMD].instance_id, can_rx[CAN_RX_MSG_IDX_ROVER_VEHICLE_CMD].recvMsg.data, can_rx[CAN_RX_MSG_IDX_ROVER_VEHICLE_CMD].recvMsg.dataLen);
#endif
}
void ancit_can_rx_notification_ROVER_ACC_PARAM(void) {
//Received data will be available in can_rx[CAN_RX_MSG_IDX_ROVER_ACC_PARAM].recvMsg 
#ifdef CAN_IDS_CONFIGURED
uint32_t actual_id = can_rx[CAN_RX_MSG_IDX_ROVER_ACC_PARAM].recvMsg.msgId;
genx_ids_check_message(actual_id, can_rx[CAN_RX_MSG_IDX_ROVER_ACC_PARAM].recvMsg.dataLen, CAN_RX_MSG_IDX_ROVER_ACC_PARAM);
if (actual_id != 0x202) { return; } /* Not our message - skip unpack */
#endif
smartwheels_rover_rover_acc_param_unpack(&can_rx_msg_rover_acc_param_instance,can_rx[CAN_RX_MSG_IDX_ROVER_ACC_PARAM].recvMsg.data,can_rx[CAN_RX_MSG_IDX_ROVER_ACC_PARAM].recvMsg.dataLen);

	ggenx.ACC_Obj = can_rx_msg_rover_acc_param_instance.acc_virt_obj_dist;
	ggenx.ACC_Dmin =can_rx_msg_rover_acc_param_instance.acc_d_min;
	ggenx.ACC_Vset =can_rx_msg_rover_acc_param_instance.acc_v_set;
#ifdef ANCIT_CAN_GATEWAY_ENABLED
ancit_can_gateway_process(0x202, can_rx[CAN_RX_MSG_IDX_ROVER_ACC_PARAM].instance_id, can_rx[CAN_RX_MSG_IDX_ROVER_ACC_PARAM].recvMsg.data, can_rx[CAN_RX_MSG_IDX_ROVER_ACC_PARAM].recvMsg.dataLen);
#endif
}


/***********************************************
 * ANCIT_CG_Rx_Notfn_End
 ***********************************************/

/***********************************************
 * ANCIT_CG_Rx_Register_Start
 ***********************************************/
// Initialize the parameters for each CAN RX message
can_rx_message_registration_params_t rx_reg[CAN_RX_MSG_MAX] = { 
//CAN_RX_MSG_IDX_ROVER_VEHICLE_CMD// 
{.idx = CAN_RX_MSG_IDX_ROVER_VEHICLE_CMD,// 
.instance_id = 0,// 
.msg_id_type = FLEXCAN_MSG_ID_STD,// 
.msg_id = 0x201,// 
.notification = ancit_can_rx_notification_ROVER_VEHICLE_CMD// 
 }, 
//CAN_RX_MSG_IDX_ROVER_ACC_PARAM// 
{.idx = CAN_RX_MSG_IDX_ROVER_ACC_PARAM,// 
.instance_id = 0,// 
.msg_id_type = FLEXCAN_MSG_ID_STD,// 
.msg_id = 0x202,// 
.notification = ancit_can_rx_notification_ROVER_ACC_PARAM// 
}

};
/***********************************************
 * ANCIT_CG_Rx_Register_End
 ***********************************************/
 
void genx_can_rx_init(void) {
	memset(can_rx, 0, sizeof(can_rx));
 
	gVars.can_rx_max = CAN_RX_MSG_MAX;
	ancit_can_rx_init();
}

 
#endif //CAN_RX_CONFIGURED
