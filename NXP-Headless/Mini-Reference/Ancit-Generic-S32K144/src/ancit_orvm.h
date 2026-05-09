/*
 * ancit_orvm.h
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
 
 * Created on: 25-03-2025
 *     Author: Narayan
 *  
 */

#ifndef ANCIT_ORVM_H_
#define ANCIT_ORVM_H_

#include "genx_config.h"

#ifdef ORVM_CONFIGURED

/**
 * @brief Mirror movement directions
 */
typedef enum {
	ORVM_MOVE_NONE,   ///< No movement
	ORVM_MOVE_LEFT,   ///< Move mirror left
	ORVM_MOVE_RIGHT,  ///< Move mirror right
	ORVM_MOVE_UP,     ///< Move mirror up
	ORVM_MOVE_DOWN    ///< Move mirror down
} ORVM_MoveDirection_t;

/**
 * @brief Internal scanning state for switch reading
 */
typedef enum {
	ORVM_STATE_SET_HIGH_CH1, ///< Activate CH1 group for scanning
	ORVM_STATE_READ_CH1,     ///< Read input state after CH1 high
	ORVM_STATE_SET_HIGH_CH2  ///< Activate CH2 group for scanning
} ORVM_State_t;

/**
 * @brief Structure to hold raw digital input values
 */
typedef struct {
	uint8_t ch1; ///< Input from channel 1
	uint8_t ch2; ///< Input from channel 2
	uint8_t ch3; ///< Input from channel 3
	uint8_t ch4; ///< Input from channel 4
} ORVM_InputState;

/**
 * @brief Direction commands for a mirror side
 */
typedef struct {
	ORVM_MoveDirection_t btn_dir_ctrl; ///< Direction from button input
	ORVM_MoveDirection_t sdv_dir_ctrl; ///< Direction from software/remote (SDV) input
} ORVM_Mirror_Side_t;

/**
 * @brief Global ORVM command structure
 */
typedef struct {
	ORVM_Mirror_Side_t left_mirror;  ///< Left mirror control commands
	ORVM_Mirror_Side_t right_mirror; ///< Right mirror control commands
} ORVM_ControlCommand_t;

// Function declarations
void orvm_init(void);
void orvm_half_bridge_enable(void);
void orvm_half_bridge_disable(void);
void orvm_read_inputs(ORVM_InputState *inputs);
void orvm_scan_switches(void);
void orvm_drive(uint8_t ch1, uint8_t ch2, uint8_t ch3);
void orvm_main(void);

#endif // ORVM_CONFIGURED

#endif /* ANCIT_ORVM_H_ */
