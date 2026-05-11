/*
 * genx_can_tx.h
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

#ifndef GENX_CAN_TX_H_
#define GENX_CAN_TX_H_
#include "genx_config.h"
#include "ancit_common.h"


#ifdef CAN_TX_CONFIGURED
#include "ancit_can_tx.h"
/***********************************************
 * ANCIT_CG_Tx_Define_Start
 ***********************************************/
 
//Total messages configured
#define CAN_TX_MSG_MAX			3
//Index for each Tx Message
#define CAN_TX_MSG_IDX_head_light_left_btn_ctrl			0
#define CAN_TX_MSG_IDX_head_light_right_btn_ctrl			1
#define CAN_TX_MSG_IDX_display                               2


/***********************************************
 * ANCIT_CG_Tx_Define_End
 ***********************************************/

extern tx_state_t sm_state;
extern can_tx_message_t can_tx_messages[CAN_TX_MSG_MAX];

void genx_can_tx_init(void);
// This macro replaces calls to genx_can_tx_main() with a direct call to ancit_can_tx_main()
#define genx_can_tx_main() ancit_can_tx_main()

#endif //CAN_TX_CONFIGURED
#endif /* GENX_CAN_TX_H_ */
