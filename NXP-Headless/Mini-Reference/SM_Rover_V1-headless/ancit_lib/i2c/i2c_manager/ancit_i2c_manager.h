/*
 * ancit_i2c_manager.h
 *
 * Base I2C Manager - Centralized I2C bus arbitration and transaction management
 * Handles priority-based scheduling for multiple I2C slave devices
 *
 * Created on: Jan 24, 2025
 * Author: Claude AI Assistant
 */

#ifndef ANCIT_I2C_MANAGER_H_
#define ANCIT_I2C_MANAGER_H_

#include <stdint.h>
#include <stdbool.h>
#include "sdk_project_config.h"
#include "genx_config.h"

#ifdef I2C_MANAGER_CONFIGURED

/*******************************************************************************
 * Configuration
 ******************************************************************************/

// Maximum number of steps in a single transaction
#define I2C_MAX_TRANSACTION_STEPS   4

// Device priorities (lower number = higher priority)
#define I2C_PRIORITY_EEPROM     0
#define I2C_PRIORITY_IMU        1
#define I2C_PRIORITY_OLED       2

// Timeouts (milliseconds)
#define I2C_TIMEOUT_EEPROM_MS   100
#define I2C_TIMEOUT_IMU_MS      100
#define I2C_TIMEOUT_OLED_MS     100

// Device I2C configurations
#define EEPROM_I2C_ADDR         0x50
#define EEPROM_I2C_BAUDRATE     400000  // 100kHz

#define IMU_I2C_ADDR            0x29
#define IMU_I2C_BAUDRATE        400000  // 400kHz

#define OLED_I2C_ADDR           0x3C
#define OLED_I2C_BAUDRATE       400000  // 400kHz

/*******************************************************************************
 * Type Definitions
 ******************************************************************************/

// Transfer type (read or write)
typedef enum {
    I2C_TRANSFER_WRITE,
    I2C_TRANSFER_READ
} i2c_transfer_type_t;

// Single step in a transaction
typedef struct {
    i2c_transfer_type_t type;    // Read or Write
    uint8_t *buffer;             // Data buffer (TX or RX)
    uint16_t size;               // Number of bytes
    bool sendStop;               // true = STOP condition, false = NO-STOP
} i2c_transfer_step_t;

// Multi-step atomic transaction
typedef struct {
    uint8_t stepCount;                                   // Total steps in transaction
    i2c_transfer_step_t steps[I2C_MAX_TRANSACTION_STEPS];
    uint8_t currentStep;                                 // Currently executing step (internal use)
} i2c_transaction_t;

// Device identifiers (also represents priority order)
typedef enum {
    I2C_DEVICE_EEPROM = 0,   // Priority 0 (highest)
    I2C_DEVICE_IMU = 1,      // Priority 1
    I2C_DEVICE_OLED = 2,     // Priority 2 (lowest)
    I2C_DEVICE_COUNT
} i2c_device_id_t;

// Device configuration
typedef struct {
    uint8_t slaveAddr;           // I2C slave address
    uint32_t baudRate;           // I2C baud rate (Hz)
    uint32_t timeoutMs;          // Transaction timeout (ms)
} i2c_device_config_t;

// Device request structure
typedef struct {
    bool requestPending;                         // Request flag
    i2c_transaction_t transaction;               // Transaction details
    void (*complete_callback)(bool success);     // Success callback
    void (*error_callback)(void);                // Error callback
} i2c_device_request_t;

// Base I2C Manager state machine states
typedef enum {
    I2C_STATE_IDLE,              // No active transaction
    I2C_STATE_ARBITRATE,         // Checking pending requests
    I2C_STATE_CONFIG_DEVICE,     // Configuring device (address, baud rate)
    I2C_STATE_EXECUTE_STEP,      // Starting I2C transfer
    I2C_STATE_WAIT_COMPLETE,     // Waiting for transfer callback
    I2C_STATE_NEXT_STEP,         // Moving to next step in transaction
    I2C_STATE_COMPLETE,          // Transaction complete, calling callback
    I2C_STATE_ERROR              // Error occurred
} i2c_manager_state_t;

// Base I2C Manager context
typedef struct {
    i2c_manager_state_t state;
    i2c_device_id_t activeDevice;                // Currently active device
    i2c_device_config_t deviceConfigs[I2C_DEVICE_COUNT];
    i2c_device_request_t deviceRequests[I2C_DEVICE_COUNT];

    bool transferComplete;                       // Set by I2C callback
    bool transferError;                          // Set by I2C callback
    uint32_t transactionStartTime;               // For timeout monitoring
    status_t lastStatus;                         // Last LPI2C operation status
} i2c_manager_t;

/*******************************************************************************
 * Public API
 ******************************************************************************/

/**
 * @brief Initialize the base I2C manager
 *
 * Initializes the I2C peripheral and configures device settings.
 * Must be called once during system initialization.
 */
void ancit_i2c_manager_init(void);

/**
 * @brief Run the I2C manager state machine
 *
 * Must be called periodically from the main loop.
 * Handles arbitration, transaction execution, and callbacks.
 */
void ancit_i2c_manager_main(void);

/**
 * @brief Submit a transaction request to the I2C manager
 *
 * @param device Device identifier
 * @param transaction Pointer to transaction structure
 * @param complete_callback Callback function called on successful completion
 * @param error_callback Callback function called on error
 * @return true if request accepted, false if device already has pending request
 */
bool ancit_i2c_submit_request(
    i2c_device_id_t device,
    i2c_transaction_t *transaction,
    void (*complete_callback)(bool success),
    void (*error_callback)(void)
);

/**
 * @brief Cancel a pending request
 *
 * @param device Device identifier
 */
void ancit_i2c_cancel_request(i2c_device_id_t device);

/**
 * @brief Check if a device has a pending request
 *
 * @param device Device identifier
 * @return true if request is pending, false otherwise
 */
bool ancit_i2c_is_request_pending(i2c_device_id_t device);

/**
 * @brief Configure I2C device parameters
 *
 * Allows application to configure device-specific I2C parameters.
 * Must be called after ancit_i2c_manager_init() and before device usage.
 *
 * @param device Device identifier
 * @param slaveAddr I2C slave address (7-bit)
 * @param baudRate I2C baud rate in Hz (e.g., 100000 for 100kHz)
 * @param timeoutMs Transaction timeout in milliseconds
 */
void ancit_i2c_configure_device(
    i2c_device_id_t device,
    uint8_t slaveAddr,
    uint32_t baudRate,
    uint32_t timeoutMs
);

/**
 * @brief I2C callback function (called by LPI2C driver)
 *
 * @param event I2C master event
 * @param userData User data (unused)
 */
void ancit_i2c_Callback(i2c_master_event_t event, void *userData);

#endif // I2C_MANAGER_CONFIGURED

#endif /* ANCIT_I2C_MANAGER_H_ */
