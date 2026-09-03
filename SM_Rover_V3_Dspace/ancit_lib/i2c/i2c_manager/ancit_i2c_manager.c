/*
 * ancit_i2c_manager.c
 *
 * Base I2C Manager Implementation
 *
 * Created on: Jan 24, 2025
 * Author: Claude AI Assistant
 */

#include "ancit_i2c_manager.h"

#ifdef I2C_MANAGER_CONFIGURED

#include "ancit_timer.h"
#include <string.h>

/*******************************************************************************
 * Private Variables
 ******************************************************************************/

// I2C manager instance
static i2c_manager_t i2cManager;

// LPI2C master state structure
static lpi2c_master_state_t lpi2cMasterState;

/*******************************************************************************
 * Private Function Prototypes
 ******************************************************************************/

static void i2c_manager_arbitrate(void);
static void i2c_manager_config_device(void);
static void i2c_manager_execute_step(void);
static void i2c_manager_check_timeout(void);
static uint32_t i2c_manager_get_current_time_ms(void);

/*******************************************************************************
 * Public Functions
 ******************************************************************************/

void ancit_i2c_manager_init(void) {
    // Initialize manager state
    memset(&i2cManager, 0, sizeof(i2c_manager_t));
    i2cManager.state = I2C_STATE_IDLE;
    i2cManager.activeDevice = I2C_DEVICE_COUNT;  // Invalid device

    // Device configurations are set by application layer using ancit_i2c_configure_device()

    // Initialize LPI2C driver (will be reconfigured per device)
    LPI2C_DRV_MasterInit(INST_LPI2C0, &lpi2c0_MasterConfig, &lpi2cMasterState);
}

void ancit_i2c_manager_main(void) {
    switch (i2cManager.state) {
    case I2C_STATE_IDLE:
        // Check if any device has a pending request
        for (uint8_t i = 0; i < I2C_DEVICE_COUNT; i++) {
            if (i2cManager.deviceRequests[i].requestPending) {
                i2cManager.state = I2C_STATE_ARBITRATE;
                break;
            }
        }
        break;

    case I2C_STATE_ARBITRATE:
        i2c_manager_arbitrate();
        break;

    case I2C_STATE_CONFIG_DEVICE:
        i2c_manager_config_device();
        break;

    case I2C_STATE_EXECUTE_STEP:
        i2c_manager_execute_step();
        break;

    case I2C_STATE_WAIT_COMPLETE:
        // Check for transfer completion
        if (i2cManager.transferComplete) {
            i2cManager.state = I2C_STATE_NEXT_STEP;
        } else if (i2cManager.transferError) {
            i2cManager.state = I2C_STATE_ERROR;
        } else {
            // Check timeout
            i2c_manager_check_timeout();
        }
        break;

    case I2C_STATE_NEXT_STEP:
        {
            i2c_device_request_t *request = &i2cManager.deviceRequests[i2cManager.activeDevice];
            request->transaction.currentStep++;

            if (request->transaction.currentStep >= request->transaction.stepCount) {
                // All steps complete
                i2cManager.state = I2C_STATE_COMPLETE;
            } else {
                // More steps to execute
                i2cManager.state = I2C_STATE_EXECUTE_STEP;
            }
        }
        break;

    case I2C_STATE_COMPLETE:
        {
            i2c_device_request_t *request = &i2cManager.deviceRequests[i2cManager.activeDevice];

            // Call completion callback
            if (request->complete_callback != NULL) {
                request->complete_callback(true);
            }

            // Clear request
            request->requestPending = false;

            // Return to idle
            i2cManager.activeDevice = I2C_DEVICE_COUNT;
            i2cManager.state = I2C_STATE_IDLE;
        }
        break;

	case I2C_STATE_ERROR:
        {
            i2c_device_request_t *request = &i2cManager.deviceRequests[i2cManager.activeDevice];

            // Call error callback
            if (request->error_callback != NULL) {
                request->error_callback();
            }

            // Clear request
            request->requestPending = false;

            // Return to idle
            i2cManager.activeDevice = I2C_DEVICE_COUNT;
            i2cManager.state = I2C_STATE_IDLE;
        }
        break;

    default:
        i2cManager.state = I2C_STATE_ERROR;
        break;
    }
}

bool ancit_i2c_submit_request(
    i2c_device_id_t device,
    i2c_transaction_t *transaction,
    void (*complete_callback)(bool success),
    void (*error_callback)(void)
) {
    // Validate parameters
    if (device >= I2C_DEVICE_COUNT || transaction == NULL) {
        return false;
    }

    // Check if device already has pending request
    if (i2cManager.deviceRequests[device].requestPending) {
        return false;
    }

    // Copy transaction data
    memcpy(&i2cManager.deviceRequests[device].transaction,
           transaction,
           sizeof(i2c_transaction_t));

    // Set callbacks
    i2cManager.deviceRequests[device].complete_callback = complete_callback;
    i2cManager.deviceRequests[device].error_callback = error_callback;

    // Mark request as pending
    i2cManager.deviceRequests[device].requestPending = true;

    return true;
}

void ancit_i2c_cancel_request(i2c_device_id_t device) {
    if (device < I2C_DEVICE_COUNT) {
        i2cManager.deviceRequests[device].requestPending = false;
    }
}

bool ancit_i2c_is_request_pending(i2c_device_id_t device) {
    if (device < I2C_DEVICE_COUNT) {
        return i2cManager.deviceRequests[device].requestPending;
    }
    return false;
}

void ancit_i2c_configure_device(
    i2c_device_id_t device,
    uint8_t slaveAddr,
    uint32_t baudRate,
    uint32_t timeoutMs
) {
    if (device < I2C_DEVICE_COUNT) {
        i2cManager.deviceConfigs[device].slaveAddr = slaveAddr;
        i2cManager.deviceConfigs[device].baudRate = baudRate;
        i2cManager.deviceConfigs[device].timeoutMs = timeoutMs;
    }
}

void ancit_i2c_Callback(i2c_master_event_t event, void *userData) {
    switch (event) {
    case I2C_MASTER_EVENT_END_TRANSFER:
        i2cManager.transferComplete = true;
        break;

    default:
        i2cManager.transferError = true;
        break;
    }

    (void)userData;
}

/*******************************************************************************
 * Private Functions
 ******************************************************************************/

static void i2c_manager_arbitrate(void) {
    // Select highest priority device with pending request
    for (uint8_t i = 0; i < I2C_DEVICE_COUNT; i++) {
        if (i2cManager.deviceRequests[i].requestPending) {
            i2cManager.activeDevice = (i2c_device_id_t)i;
            i2cManager.state = I2C_STATE_CONFIG_DEVICE;
            return;
        }
    }

    // No pending requests found (shouldn't happen)
    i2cManager.state = I2C_STATE_IDLE;
}

static void i2c_manager_config_device(void) {
    i2c_device_config_t *config = &i2cManager.deviceConfigs[i2cManager.activeDevice];
    i2c_device_request_t *request = &i2cManager.deviceRequests[i2cManager.activeDevice];

    // Set slave address
    LPI2C_DRV_MasterSetSlaveAddr(INST_LPI2C0, config->slaveAddr, false);

    // Set baud rate
    lpi2c_baud_rate_params_t baudRate;
    baudRate.baudRate = config->baudRate;
    (void)LPI2C_DRV_MasterSetBaudRate(INST_LPI2C0, LPI2C_FAST_MODE, baudRate);

    // Initialize transaction
    request->transaction.currentStep = 0;

    // Start timeout timer
    i2cManager.transactionStartTime = i2c_manager_get_current_time_ms();

    // Clear flags
    i2cManager.transferComplete = false;
    i2cManager.transferError = false;

    // Proceed to execute first step
    i2cManager.state = I2C_STATE_EXECUTE_STEP;
}

static void i2c_manager_execute_step(void) {
    i2c_device_request_t *request = &i2cManager.deviceRequests[i2cManager.activeDevice];
    i2c_transfer_step_t *step = &request->transaction.steps[request->transaction.currentStep];

    // Clear completion flags
    i2cManager.transferComplete = false;
    i2cManager.transferError = false;

    // Execute based on transfer type
    if (step->type == I2C_TRANSFER_WRITE) {
        i2cManager.lastStatus = LPI2C_DRV_MasterSendData(
            INST_LPI2C0,
            step->buffer,
            step->size,
            step->sendStop
        );
    } else {  // I2C_TRANSFER_READ
        i2cManager.lastStatus = LPI2C_DRV_MasterReceiveData(
            INST_LPI2C0,
            step->buffer,
            step->size,
            step->sendStop
        );
    }

    // Check if operation started successfully
    if (i2cManager.lastStatus == STATUS_SUCCESS) {
        i2cManager.state = I2C_STATE_WAIT_COMPLETE;
    } else {
        i2cManager.state = I2C_STATE_ERROR;
    }
}

static void i2c_manager_check_timeout(void) {
    i2c_device_config_t *config = &i2cManager.deviceConfigs[i2cManager.activeDevice];
    uint32_t currentTime = i2c_manager_get_current_time_ms();
    uint32_t elapsedTime = currentTime - i2cManager.transactionStartTime;

    if (elapsedTime >= config->timeoutMs) {
        // Timeout occurred
        i2cManager.state = I2C_STATE_ERROR;
    }
}

static uint32_t i2c_manager_get_current_time_ms(void) {
    // Use existing timer function from ancit_timer.h
    return ancit_GetTick();
}

#endif // I2C_MANAGER_CONFIGURED
