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
 
 * Created on: 01-07-2025
 *     Author: Narayan
 *  
 */
#include <string.h>
#include "genx_can_rx.h"

#ifdef CAN_RX_CONFIGURED
g_struct_can_rx_t can_rx[CAN_RX_MSG_MAX];

#include "smart_wheels_hl.h"
#include "ancit_head_light.h"

struct smart_wheels_hl_head_light_left_sdv_ctrl_t can_rx_msg_head_light_left_sdv_ctrl_instance;
struct smart_wheels_hl_head_light_right_sdv_ctrl_t can_rx_msg_head_light_right_sdv_ctrl_instance;

extern volatile finalLightStatus_t finalLightStatus_right_sdv;
extern volatile finalLightStatus_t finalLightStatus_left_sdv;

/***********************************************
 * ANCIT_CG_Rx_Notfn_Start
 ***********************************************/
 
void ancit_can_rx_notification_head_light_left_sdv_ctrl(void) {
//Received data will be available in can_rx[CAN_RX_MSG_IDX_head_light_left_sdv_ctrl].recvMsg 
smart_wheels_hl_head_light_left_sdv_ctrl_unpack(&can_rx_msg_head_light_left_sdv_ctrl_instance,can_rx[CAN_RX_MSG_IDX_head_light_left_sdv_ctrl].recvMsg.data,can_rx[CAN_RX_MSG_IDX_head_light_left_sdv_ctrl].recvMsg.dataLen);
finalLightStatus_left_sdv.parking_light = can_rx_msg_head_light_left_sdv_ctrl_instance.parking_light;


}
void ancit_can_rx_notification_head_light_right_sdv_ctrl(void) {
//Received data will be available in can_rx[CAN_RX_MSG_IDX_head_light_right_sdv_ctrl].recvMsg 
smart_wheels_hl_head_light_right_sdv_ctrl_unpack(&can_rx_msg_head_light_right_sdv_ctrl_instance,can_rx[CAN_RX_MSG_IDX_head_light_right_sdv_ctrl].recvMsg.data,can_rx[CAN_RX_MSG_IDX_head_light_right_sdv_ctrl].recvMsg.dataLen);
finalLightStatus_right_sdv.parking_light = can_rx_msg_head_light_right_sdv_ctrl_instance.parking_light;
finalLightStatus_right_sdv.indicator_enabled = can_rx_msg_head_light_right_sdv_ctrl_instance.indicator_light;
// finalLightStatus_right_sdv.indicator_enabled = can_rx_msg_head_light_right_sdv_ctrl_instance.indicator_light;
finalLightStatus_right_sdv.head_light_beam = can_rx_msg_head_light_right_sdv_ctrl_instance.head_light_beam;

}


/***********************************************
 * ANCIT_CG_Rx_Notfn_End
 ***********************************************/

/***********************************************
 * ANCIT_CG_Rx_Register_Start
 ***********************************************/
// Initialize the parameters for each CAN RX message
can_rx_message_registration_params_t rx_reg[CAN_RX_MSG_MAX] = { 
//CAN_RX_MSG_IDX_head_light_left_sdv_ctrl// 
{.idx = CAN_RX_MSG_IDX_head_light_left_sdv_ctrl,// 
.msg_id = 0x325,// 
.notification = ancit_can_rx_notification_head_light_left_sdv_ctrl// 
 }, 
//CAN_RX_MSG_IDX_head_light_right_sdv_ctrl// 
{.idx = CAN_RX_MSG_IDX_head_light_right_sdv_ctrl,// 
.msg_id = 0x326,// 
.notification = ancit_can_rx_notification_head_light_right_sdv_ctrl// 
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
