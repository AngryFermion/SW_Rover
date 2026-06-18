/*
 * genx_i2c_manager.h
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

 * Created on: 30-01-2025
 *     Author: ANCIT
 *
 */

#ifndef GENX_I2C_MANAGER_H_
#define GENX_I2C_MANAGER_H_

#include "genx_config.h"

#ifdef I2C_MANAGER_CONFIGURED

/***********************************************
 * Public API
 ***********************************************/

/**
 * @brief Initialize I2C Manager with project-specific device configuration
 *
 * Initializes the base I2C manager and configures all I2C devices
 * used in this project (EEPROM, IMU, OLED, etc.)
 *
 * Must be called once during system initialization before any I2C device init.
 */
void genx_i2c_manager_init(void);

/**
 * @brief Run I2C Manager main loop
 *
 * Must be called periodically from the main loop.
 * Delegates to the base I2C manager state machine.
 *
 * Non-blocking - returns immediately.
 */
void genx_i2c_manager_main(void);

#endif // I2C_MANAGER_CONFIGURED

#endif /* GENX_I2C_MANAGER_H_ */
