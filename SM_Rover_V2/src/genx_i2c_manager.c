/*
 * genx_i2c_manager.c
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

#include "genx_i2c_manager.h"

#ifdef I2C_MANAGER_CONFIGURED

#include "ancit_i2c_manager.h"

#ifdef OLED_DISPLAY_CONFIGURED
#include "genx_oled.h"
#endif

#ifdef IMU_SENSOR_CONFIGURED
#include "genx_imu.h"
#endif

#ifdef NVM_EEPROM_CONFIGURED
#include "genx_eeprom.h"
#include "ancit_eeprom.h"
#endif

/***********************************************
 * Public API Implementation
 ***********************************************/

void genx_i2c_manager_init(void) {
	// Initialize base I2C manager
	ancit_i2c_manager_init();

	// Configure project-specific I2C devices
	// Device configurations are set based on what's enabled in genx_config.h

#ifdef NVM_EEPROM_CONFIGURED
	// Configure EEPROM (24C04)
	ancit_i2c_configure_device(
		I2C_DEVICE_EEPROM,
		EEPROM_I2C_ADDR,      // 0x50
		EEPROM_I2C_BAUDRATE,  // 100kHz
		I2C_TIMEOUT_EEPROM_MS // 100ms
	);
#endif

#ifdef IMU_SENSOR_CONFIGURED
	// Configure IMU (BNO055)
	ancit_i2c_configure_device(
		I2C_DEVICE_IMU,
		IMU_I2C_ADDR,         // 0x29
		IMU_I2C_BAUDRATE,     // 400kHz
		I2C_TIMEOUT_IMU_MS    // 100ms
	);
#endif

#ifdef OLED_DISPLAY_CONFIGURED
	// Configure OLED (SSD1306)
	ancit_i2c_configure_device(
		I2C_DEVICE_OLED,
		OLED_I2C_ADDR,        // 0x3C
		OLED_I2C_BAUDRATE,    // 400kHz
		I2C_TIMEOUT_OLED_MS   // 100ms
	);
#endif

	// Initialize all I2C devices
	// These must be called after I2C manager and device configuration

#ifdef NVM_EEPROM_CONFIGURED
	// Initialize EEPROM driver
	ancit_eeprom_init();
#endif

#ifdef IMU_SENSOR_CONFIGURED
	// Initialize IMU test module (includes driver init)
	ancit_delay_ms(100); // Wait for IMU to power up
	genx_imu_init();
#endif

#ifdef OLED_DISPLAY_CONFIGURED
	// Initialize OLED display
	genx_oled_init();
#endif
}

void genx_i2c_manager_main(void) {
	// Run base I2C manager state machine
	ancit_i2c_manager_main();

	// Run all I2C device main loops

#ifdef NVM_EEPROM_CONFIGURED
	// Run EEPROM driver state machine (includes block management)
	ancit_eeprom_main();
#endif

#ifdef IMU_SENSOR_CONFIGURED
	// Run IMU test module (includes driver state machine)
	genx_imu_main();
#endif

#ifdef OLED_DISPLAY_CONFIGURED
	// Run OLED display updates
	genx_oled_main();
#endif
}

#endif // I2C_MANAGER_CONFIGURED
