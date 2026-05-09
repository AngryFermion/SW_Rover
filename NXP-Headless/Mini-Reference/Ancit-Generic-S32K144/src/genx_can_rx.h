/*
 * genx_can_rx.h
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

#ifndef GENX_CAN_RX_H_
#define GENX_CAN_RX_H_
#include "genx_config.h"
#include "ancit_common.h"

#ifdef CAN_RX_CONFIGURED
#include "ancit_can_rx.h"
/***********************************************
* ANCIT_CG_Define_Start
***********************************************/

//Total messages configured
#define CAN_RX_MSG_MAX			2
//Index for each Rx Message
#define CAN_RX_MSG_IDX_head_light_left_sdv_ctrl			0
#define CAN_RX_MSG_IDX_head_light_right_sdv_ctrl			1


/***********************************************
* ANCIT_CG_Define_End
***********************************************/

extern can_rx_message_registration_params_t rx_reg[CAN_RX_MSG_MAX];
extern g_struct_can_rx_t can_rx[CAN_RX_MSG_MAX];

void genx_can_rx_init(void);
// This macro replaces calls to genx_can_rx_main() with a direct call to ancit_can_rx_main()
#define genx_can_rx_main() ancit_can_rx_main()


#endif //CAN_RX_CONFIGURED
#endif /* GENX_CAN_RX_H_ */
