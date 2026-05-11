/*
 * ancit_orvm.c
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

#include <ancit_orvm.h>
#include "ancit_common.h"
#include "ancit_timer.h"
#include "ancit_driver_digital_io.h"

#ifdef ORVM_CONFIGURED

typedef enum {
	ORVM_STATE_CD_IDLE = 0,					// idle state
	ORVM_STATE_CD_DISABLE_DRIVERS,          // orvm_disable_all_drivers
	ORVM_STATE_CD_ENABLE_DRIVERS,      // orvm_enable_low and high side_drivers
} orvm_state_cd_t;

static orvm_state_cd_t orvm_state_cd = ORVM_STATE_CD_IDLE;

ORVM_ControlCommand_t orvm;

/**
 * @brief Enables the ORVM half-bridge drivers.
 *
 * Sets the shared SLEEP pin HIGH to wake both half-bridges from low-power mode.
 * This must be done to drive any outputs.
 * The SLEEP signal is active-low, so driving it HIGH enables the bridges.
 */
void orvm_half_bridge_enable(void) {
	// SLEEP is active-low; driving HIGH enables the half-bridges
	// Both H-bridges are controlled by a common SLEEP line
	PINS_DRV_WritePin(DO_HB_SLEEP_PORT, DO_HB_SLEEP_PIN, HIGH);
}

/**
 * @brief Disables the ORVM half-bridge drivers.
 *
 * This function sets all drive outputs to LOW to ensure no active drive,
 * then pulls the SLEEP pin LOW to place both half-bridges into low-power or inactive mode.
 * It should be used to safely shut down mirror drive control.
 */
void orvm_half_bridge_disable(void) {
	orvm_drive(LOW, LOW, LOW); // Ensure all outputs are inactive before disabling
	PINS_DRV_WritePin(DO_HB_SLEEP_PORT, DO_HB_SLEEP_PIN, LOW); // Enter sleep mode
}

/**
 * @brief Drives the ORVM control outputs.
 *
 * This function sets the output states for the three half-bridge control lines.
 * If any of the channels is active (i.e., not all are LOW), it enables the half-bridge driver.
 *
 * @param ch1 Output state for channel 1 (HIGH or LOW)
 * @param ch2 Output state for channel 2 (HIGH or LOW)
 * @param ch3 Output state for channel 3 (HIGH or LOW)
 */
void orvm_drive(uint8_t ch1, uint8_t ch2, uint8_t ch3) {
	// Drive output pins
	PINS_DRV_WritePin(DO_HALFB_CH1_PORT, DO_HALFB_CH1_PIN, ch1);
	PINS_DRV_WritePin(DO_HALFB_CH2_PORT, DO_HALFB_CH2_PIN, ch2);
	PINS_DRV_WritePin(DO_HALFB_CH3_PORT, DO_HALFB_CH3_PIN, ch3);

	// If any channel is active (not all LOW), enable the half-bridge
	if (!(ch1 == LOW && ch2 == LOW && ch3 == LOW)) {
		orvm_half_bridge_enable();
	}
}

/**
 * @brief Initializes the ORVM system state and hardware.
 *
 * This function sets the initial direction commands for both left and right mirrors to NONE,
 * sets the control state machine to IDLE, and initializes the half-bridge drivers.
 *
 * It ensures both H-bridges are first disabled safely.
 * This should be called once during system startup.
 */
void orvm_init(void) {
	// Initialize mirror direction states to NONE
	orvm.left_mirror.btn_dir_ctrl = ORVM_MOVE_NONE;
	orvm.left_mirror.sdv_dir_ctrl = ORVM_MOVE_NONE;
	orvm.right_mirror.btn_dir_ctrl = ORVM_MOVE_NONE;
	orvm.right_mirror.sdv_dir_ctrl = ORVM_MOVE_NONE;

	// Set ORVM control state to IDLE
	orvm_state_cd = ORVM_STATE_CD_IDLE;

	// Initialize half-bridge drivers:
	// First disable to ensure outputs are off
	orvm_half_bridge_disable();
}

/**
 * @brief Reads the current digital input states for the ORVM control channels.
 *
 * This function samples the digital input pins connected to CH1 through CH4
 * and stores their logical levels into the provided ORVM_InputState structure.
 *
 * @param[in,out] inputs Pointer to ORVM_InputState structure where input values will be stored.
 */
void orvm_read_inputs(ORVM_InputState *inputs) {
	inputs->ch1 = ancit_dd_read_pin(DI_PD_CH1_PORT, DI_PD_CH1_PIN); // Read input for channel 1
	inputs->ch2 = ancit_dd_read_pin(DI_PD_CH2_PORT, DI_PD_CH2_PIN); // Read input for channel 2
	inputs->ch3 = ancit_dd_read_pin(DI_PD_CH3_PORT, DI_PD_CH3_PIN); // Read input for channel 3
	inputs->ch4 = ancit_dd_read_pin(DI_PD_CH4_PORT, DI_PD_CH4_PIN); // Read input for channel 4
}

/**
 * @brief Scans ORVM control switches and updates direction commands.
 *
 * This function uses a state machine to sequentially scan two input groups (CH1 and CH2)
 * by enabling high-side drivers and reading the digital input states.
 * It detects user switch actions for left/right mirrors and sets the direction commands accordingly.
 *
 * Note: Should be called periodically (e.g., every 10ms).
 */
void orvm_scan_switches(void) {
	// Static variables retain their values between calls
	static ORVM_ControlCommand_t orvm_local;
	static ORVM_State_t orvm_state = ORVM_STATE_SET_HIGH_CH1;

	switch (orvm_state) {
	case ORVM_STATE_SET_HIGH_CH1: {
		// Step 1: Activate scan group 1 (CH1 high)
		PINS_DRV_WritePin(DO_HS_CH1_PORT, DO_HS_CH1_PIN, HIGH);

		// Clear local direction commands before reading
		orvm_local.left_mirror.btn_dir_ctrl = ORVM_MOVE_NONE;
		orvm_local.right_mirror.btn_dir_ctrl = ORVM_MOVE_NONE;

		orvm_state = ORVM_STATE_READ_CH1;
		break;
	}

	case ORVM_STATE_READ_CH1: {
		// Step 2: Read inputs while CH1 is high
		ORVM_InputState inputs;
		orvm_read_inputs(&inputs);

		// Detect LEFT and UP directions while CH1 is high
		if (inputs.ch1 == HIGH) {
			orvm_local.left_mirror.btn_dir_ctrl = ORVM_MOVE_LEFT;
		} else if (inputs.ch2 == HIGH) {
			orvm_local.left_mirror.btn_dir_ctrl = ORVM_MOVE_UP;
		} else if (inputs.ch3 == HIGH) {
			orvm_local.right_mirror.btn_dir_ctrl = ORVM_MOVE_UP;
		} else if (inputs.ch4 == HIGH) {
			orvm_local.right_mirror.btn_dir_ctrl = ORVM_MOVE_LEFT;
		}

		// Deactivate CH1 scan group
		PINS_DRV_WritePin(DO_HS_CH1_PORT, DO_HS_CH1_PIN, LOW);

		// Prepare CH2 for next scan group
		PINS_DRV_WritePin(DO_HS_CH2_PORT, DO_HS_CH2_PIN, HIGH);
		orvm_state = ORVM_STATE_SET_HIGH_CH2;
		break;
	}

	case ORVM_STATE_SET_HIGH_CH2: {
		// Step 3: Read inputs while CH2 is high
		ORVM_InputState inputs;
		orvm_read_inputs(&inputs);

		// Detect RIGHT and DOWN directions while CH2 is high
		if (inputs.ch1 == HIGH) {
			orvm_local.left_mirror.btn_dir_ctrl = ORVM_MOVE_RIGHT;
		} else if (inputs.ch2 == HIGH) {
			orvm_local.left_mirror.btn_dir_ctrl = ORVM_MOVE_DOWN;
		} else if (inputs.ch3 == HIGH) {
			orvm_local.right_mirror.btn_dir_ctrl = ORVM_MOVE_DOWN;
		} else if (inputs.ch4 == HIGH) {
			orvm_local.right_mirror.btn_dir_ctrl = ORVM_MOVE_RIGHT;
		}

		// Deactivate CH2 scan group
		PINS_DRV_WritePin(DO_HS_CH2_PORT, DO_HS_CH2_PIN, LOW);

		// Update global command with detected direction
		orvm.left_mirror.btn_dir_ctrl = orvm_local.left_mirror.btn_dir_ctrl;
		orvm.right_mirror.btn_dir_ctrl = orvm_local.right_mirror.btn_dir_ctrl;

		// Restart scan from CH1
		orvm_state = ORVM_STATE_SET_HIGH_CH1;
		break;
	}

	default:
		orvm_state = ORVM_STATE_SET_HIGH_CH1;
		break;
	}
}

/**
 * @brief ORVM direction state machine.
 *
 * This function determines the desired mirror movement direction
 * based on button and SDV commands (button has higher priority),
 * and transitions through states to safely disable and re-enable
 * the half-bridge drivers with updated direction control.
 *
 * Should be called periodically (e.g., every 10ms).
 */
void orvm_change_direction_sm(void) {
    static ORVM_MoveDirection_t prev_orvm_direction = ORVM_MOVE_NONE;


//Narayan - this has to be changed to right mirror
    // Determine effective direction: button input has priority over SDV
    ORVM_MoveDirection_t effective_dir =
        (orvm.left_mirror.btn_dir_ctrl != ORVM_MOVE_NONE) ?
        orvm.left_mirror.btn_dir_ctrl :
        orvm.left_mirror.sdv_dir_ctrl;

    switch (orvm_state_cd) {
    case ORVM_STATE_CD_IDLE:
        // If direction changed, proceed to disable before switching
        if (prev_orvm_direction != effective_dir) {
            prev_orvm_direction = effective_dir;
            orvm_state_cd = ORVM_STATE_CD_DISABLE_DRIVERS;
        }
        break;

    case ORVM_STATE_CD_DISABLE_DRIVERS:
        // Safely disable all outputs
        orvm_half_bridge_disable();

        if (effective_dir == ORVM_MOVE_NONE) {
            // Nothing to do — remain idle
            orvm_state_cd = ORVM_STATE_CD_IDLE;
        } else {
            // Proceed to enable appropriate drive direction
            orvm_state_cd = ORVM_STATE_CD_ENABLE_DRIVERS;
        }
        break;

    case ORVM_STATE_CD_ENABLE_DRIVERS:
        // Drive in the new direction based on effective command
        switch (effective_dir) {
        case ORVM_MOVE_LEFT:
            orvm_drive(HIGH, LOW, LOW);    // CH1=1, CH2=0, CH3=0
            break;

        case ORVM_MOVE_RIGHT:
            orvm_drive(LOW, HIGH, HIGH);   // CH1=0, CH2=1, CH3=1
            break;

        case ORVM_MOVE_UP:
            orvm_drive(LOW, HIGH, LOW);    // CH1=0, CH2=1, CH3=0
            break;

        case ORVM_MOVE_DOWN:
            orvm_drive(HIGH, LOW, HIGH);   // CH1=1, CH2=0, CH3=1
            break;

        default:
            orvm_drive(LOW, LOW, LOW);     // Fallback: disable all
            break;
        }

        // Return to idle after applying drive
        orvm_state_cd = ORVM_STATE_CD_IDLE;
        break;

    default:
        orvm_state_cd = ORVM_STATE_CD_IDLE;
        break;
    }
}


/**
 * @brief Main periodic function for ORVM control.
 *
 * This function scans the input switches and updates the mirror motor
 * direction accordingly. It should be called every 10 ms from a scheduler or timer.
 *
 * Responsibilities:
 * - Scans the current state of mirror control switches
 * - Determines and drives the motor in the appropriate direction
 */
void orvm_main(void) {
	orvm_scan_switches();        // Read and update current switch states
	orvm_change_direction_sm(); // Update motor direction based on switch state logic
}

#endif //ORVM_CONFIGURED
