/*
 * ancit_oled_handler.h
 *
 * OLED Element Handler - Reusable logic for managing OLED display elements
 * This module handles the refresh logic and element management.
 * Configuration comes from genx_oled.c (tool-generated)
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

 * Created on: 27-01-2025
 *     Author: ANCIT
 *
 */

#ifndef ANCIT_OLED_HANDLER_H_
#define ANCIT_OLED_HANDLER_H_

#include <stdint.h>
#include <stdbool.h>
#include "ancit_oled_driver.h"

/***********************************************
 * Type Definitions
 ***********************************************/

/**
 * @brief OLED element types
 */
typedef enum {
    OLED_ELEMENT_CONST_TEXT,    // Static text that never changes
    OLED_ELEMENT_VARIABLE,      // Dynamic value updated via callback
    OLED_ELEMENT_DRAWING        // Reserved for future: shapes, bitmaps, etc.
} oled_element_type_t;

/**
 * @brief Font type enumeration
 *
 * Maps to the available SSD1306 fonts.
 * Make sure the corresponding font is enabled in ssd1306_conf.h
 */
typedef enum {
    OLED_FONT_6x8,      // SSD1306_INCLUDE_FONT_6x8
    OLED_FONT_7x10,     // SSD1306_INCLUDE_FONT_7x10
    OLED_FONT_11x18,    // SSD1306_INCLUDE_FONT_11x18
    OLED_FONT_16x26,    // SSD1306_INCLUDE_FONT_16x26
    OLED_FONT_16x24,    // SSD1306_INCLUDE_FONT_16x24
    OLED_FONT_16x15     // SSD1306_INCLUDE_FONT_16x15
} oled_font_type_t;

/**
 * @brief Value formatter callback function
 *
 * This callback is called periodically to get the current value of a variable
 * and format it as a string for display.
 *
 * @param buffer Output buffer to write the formatted string
 * @param bufferSize Size of the output buffer
 *
 * Example:
 *   void format_can_speed(char* buf, uint8_t size) {
 *       uint16_t speed = get_vehicle_speed(); // Your function to get CAN data
 *       snprintf(buf, size, "Speed:%3d", speed);
 *   }
 */
typedef void (*oled_value_formatter_t)(char* buffer, uint8_t bufferSize);

/**
 * @brief OLED element registration structure
 *
 * Configuration for one display element (text or variable).
 * This structure is populated by the code generation tool in genx_oled.c
 */
typedef struct {
    oled_element_type_t type;       // Element type
    uint8_t x;                      // X position (0-127)
    uint8_t y;                      // Y position (0-63)
    oled_font_type_t font;          // Font to use

    // For OLED_ELEMENT_CONST_TEXT
    const char* const_text;         // Pointer to constant text string

    // For OLED_ELEMENT_VARIABLE
    oled_value_formatter_t value_callback;  // Callback to get and format value
    uint8_t buffer_size;                     // Buffer size for formatting
} oled_element_reg_t;

/**
 * @brief Runtime state for each OLED element
 */
typedef struct
{
    bool updated;  // Set when this element has been updated in current cycle
} oled_element_state_t;

/***********************************************
 * Public API
 ***********************************************/

/**
 * @brief Initialize OLED handler
 *
 * Initializes the OLED driver, clears the screen, and draws elements.
 * Must be called once during system init.
 *
 * @param elements Pointer to element registration array
 * @param element_states Pointer to element runtime state array (same size as elements)
 * @param element_count Number of elements in the array
 * @param refresh_interval_ms Refresh interval in milliseconds
 *
 * NOTE: This function contains BLOCKING operations and may take 100-200ms.
 */
void ancit_oled_handler_init(const oled_element_reg_t* elements,
                              oled_element_state_t* element_states,
                              uint8_t element_count,
                              uint16_t refresh_interval_ms);

/**
 * @brief OLED handler main loop function
 *
 * Must be called periodically from the main loop.
 * - Runs the OLED state machine (ssd1306_Main)
 * - Updates variable elements at configured refresh rate
 * - Triggers screen updates when needed
 *
 * Non-blocking - returns immediately.
 */
void ancit_oled_handler_main(void);

/**
 * @brief Force immediate update of all variable elements
 *
 * Normally, variables are updated at the configured refresh rate.
 * Call this function to force an immediate update of all variables
 * and screen refresh (useful after important events).
 */
void ancit_oled_handler_force_update(void);

/**
 * @brief Check if OLED is currently busy
 *
 * @return true if OLED is processing an update, false if idle
 */
bool ancit_oled_handler_is_busy(void);

/**
 * @brief Convert font type enum to actual font structure
 *
 * @param font_type Font type enum
 * @return Corresponding SSD1306_Font_t structure
 */
SSD1306_Font_t ancit_oled_get_font(oled_font_type_t font_type);

#endif /* ANCIT_OLED_HANDLER_H_ */
