/*
 * ancit_oled_handler.c
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

#include "ancit_oled_handler.h"
#include "ancit_timer.h"
#include "ssd1306_fonts.h"
#include <stdio.h>
#include <string.h>

/***********************************************
 * Configuration
 ***********************************************/

/**
 * @brief Maximum buffer size for formatted variable strings
 */
#ifndef OLED_VALUE_BUFFER_SIZE
#define OLED_VALUE_BUFFER_SIZE      32
#endif

/**
 * @brief Number of main loop calls to wait between variable updates
 */
#ifndef OLED_UPDATE_DELAY_COUNT
#define OLED_UPDATE_DELAY_COUNT     25
#endif

/***********************************************
 * Private Variables
 ***********************************************/

// Configuration (set during init)
static const oled_element_reg_t* g_elements = NULL;
static oled_element_state_t* g_element_states = NULL;
static uint8_t g_element_count = 0;
static uint16_t g_refresh_interval_ms = 100;

// Refresh timing control
static uint32_t last_refresh_time = 0;
static bool refresh_requested = false;  // Set every 100ms, cleared when all variables updated and I2C write done

// Round-robin update tracking
static uint8_t current_update_index = 0;
static uint8_t variable_element_count = 0;  // Count of variable elements only
static uint8_t update_delay_counter = 0;    // Counter for delaying between variable updates

// Value formatting buffer (shared for all variables)
static char value_buffer[OLED_VALUE_BUFFER_SIZE];

/***********************************************
 * Update State Helper Functions
 ***********************************************/

/**
 * @brief Check if all variable elements have been updated
 */
static bool all_variables_updated(void)
{
    uint8_t i;
    uint8_t updated_count;

    if (variable_element_count == 0) {
        return true;
    }

    updated_count = 0;
    for (i = 0; i < g_element_count; i++) {
        if (g_elements[i].type == OLED_ELEMENT_VARIABLE) {
            if (g_element_states[i].updated) {
                updated_count++;
            }
        }
    }

    return (updated_count >= variable_element_count);
}

/**
 * @brief Clear all update flags
 */
static void clear_all_updated_flags(void)
{
    uint8_t i;

    for (i = 0; i < g_element_count; i++) {
        g_element_states[i].updated = false;
    }
}

/***********************************************
 * Font Mapping Helper
 ***********************************************/

/**
 * @brief Convert font type enum to actual font structure
 */
SSD1306_Font_t ancit_oled_get_font(oled_font_type_t font_type) {
    switch (font_type) {
#ifdef SSD1306_INCLUDE_FONT_6x8
    case OLED_FONT_6x8:
        return Font_6x8;
#endif
#ifdef SSD1306_INCLUDE_FONT_7x10
    case OLED_FONT_7x10:
        return Font_7x10;
#endif
#ifdef SSD1306_INCLUDE_FONT_11x18
    case OLED_FONT_11x18:
        return Font_11x18;
#endif
#ifdef SSD1306_INCLUDE_FONT_16x26
    case OLED_FONT_16x26:
        return Font_16x26;
#endif
#ifdef SSD1306_INCLUDE_FONT_16x24
    case OLED_FONT_16x24:
        return Font_16x24;
#endif
#ifdef SSD1306_INCLUDE_FONT_16x15
    case OLED_FONT_16x15:
        return Font_16x15;
#endif
    default:
        // Default to Font_7x10 if available, otherwise first available font
#ifdef SSD1306_INCLUDE_FONT_7x10
        return Font_7x10;
#elif defined(SSD1306_INCLUDE_FONT_6x8)
        return Font_6x8;
#elif defined(SSD1306_INCLUDE_FONT_11x18)
        return Font_11x18;
#else
        return Font_16x26;  // Last resort
#endif
    }
}

/***********************************************
 * Private Functions
 ***********************************************/

/**
 * @brief Draw all constant text elements
 */
static void ancit_oled_draw_const_text(void) {
    if (g_elements == NULL) {
        return;
    }

    for (uint8_t i = 0; i < g_element_count; i++) {
        if (g_elements[i].type == OLED_ELEMENT_CONST_TEXT) {
            SSD1306_Font_t font = ancit_oled_get_font(g_elements[i].font);
            ssd1306_WriteString(
                g_elements[i].const_text,
                font,
                g_elements[i].x,
                g_elements[i].y
            );
        }
    }
}

/**
 * @brief Update all variable elements (used during init)
 */
static void ancit_oled_update_all_variables(void)
{
    uint8_t i;

    if (g_elements == NULL) {
        return;
    }

    for (i = 0; i < g_element_count; i++) {
        if (g_elements[i].type == OLED_ELEMENT_VARIABLE) {
            // Call the user's formatter callback to get the current value
            if (g_elements[i].value_callback != NULL) {
                // Clear the buffer
                memset(value_buffer, 0, OLED_VALUE_BUFFER_SIZE);

                // Get formatted value from callback
                g_elements[i].value_callback(
                    value_buffer,
                    g_elements[i].buffer_size
                );

                // Draw the formatted value on screen
                SSD1306_Font_t font = ancit_oled_get_font(g_elements[i].font);
                ssd1306_WriteString(
                    value_buffer,
                    font,
                    g_elements[i].x,
                    g_elements[i].y
                );
            }
        }
    }
}

/**
 * @brief Update one variable element (round-robin)
 *
 * This function updates only one variable element per call to distribute
 * CPU load over time. It uses a round-robin approach to cycle through
 * all variable elements. Updates the buffer but doesn't trigger screen refresh.
 * Screen refresh happens only when all variables are updated.
 *
 * @return true if this was the last variable (all now updated), false otherwise
 */
static bool ancit_oled_update_one_variable(void)
{
    uint8_t search_count;
    uint8_t i;

    if (g_elements == NULL || g_element_count == 0) {
        return false;
    }

    // Search for next variable element (with wraparound protection)
    search_count = 0;
    while (search_count < g_element_count) {
        i = current_update_index;

        // Move to next index for next call
        current_update_index++;
        if (current_update_index >= g_element_count) {
            current_update_index = 0;
        }

        // If this is a variable element, update it
        if (g_elements[i].type == OLED_ELEMENT_VARIABLE) {
            if (g_elements[i].value_callback != NULL) {
                // Clear the buffer
                memset(value_buffer, 0, OLED_VALUE_BUFFER_SIZE);

                // Get formatted value from callback
                g_elements[i].value_callback(
                    value_buffer,
                    g_elements[i].buffer_size
                );

                // Draw the formatted value to buffer (not yet to screen)
                SSD1306_Font_t font = ancit_oled_get_font(g_elements[i].font);
                ssd1306_WriteString(
                    value_buffer,
                    font,
                    g_elements[i].x,
                    g_elements[i].y
                );

                // Mark this element as updated
                g_element_states[i].updated = true;
            }
            // Check if all variables are now updated
            return all_variables_updated();
        }

        search_count++;
    }

    return false;
}

/***********************************************
 * Public API Implementation
 ***********************************************/

void ancit_oled_handler_init(const oled_element_reg_t* elements,
                              oled_element_state_t* element_states,
                              uint8_t element_count,
                              uint16_t refresh_interval_ms)
{
    uint8_t i;

    // Store configuration
    g_elements = elements;
    g_element_states = element_states;
    g_element_count = element_count;
    g_refresh_interval_ms = refresh_interval_ms;

    // Count variable elements
    variable_element_count = 0;
    for (i = 0; i < element_count; i++) {
        if (elements[i].type == OLED_ELEMENT_VARIABLE) {
            variable_element_count++;
        }
    }

    // Clear update flags
    clear_all_updated_flags();

    // Initialize OLED driver (blocking - takes ~100-200ms)
    ssd1306_Init();

    // Clear the screen
    ssd1306_Clear();

    // Draw all constant text elements
    ancit_oled_draw_const_text();

    // Draw initial variable values (all at once during init)
    ancit_oled_update_all_variables();

    // Update screen (blocking to ensure init display is shown)
    ssd1306_UpdateScreenBlocking();

    // Initialize timing
    last_refresh_time = ancit_GetTick();
    refresh_requested = false;
}

void ancit_oled_handler_main(void)
{
    uint32_t current_time;
    bool all_updated;

    // Always run the OLED state machine (handles I2C transfers)
    ssd1306_Main();

    // Check if 100ms has elapsed - if yes, set refresh_requested flag
    current_time = ancit_GetTick();
    if ((current_time - last_refresh_time) >= g_refresh_interval_ms)
    {
        refresh_requested = true;
        last_refresh_time = current_time;
        update_delay_counter = 0;  // Reset delay counter when new refresh cycle starts
    }

    // If refresh is requested, update variables one-by-one with delay
    if (refresh_requested && !ssd1306_IsBusy())
    {
        // Increment delay counter
        update_delay_counter++;

        // Update ONE variable element only when delay count is reached
        if (update_delay_counter >= OLED_UPDATE_DELAY_COUNT)
        {
            update_delay_counter = 0;  // Reset counter for next variable
            all_updated = ancit_oled_update_one_variable();

            // When all variables are updated, trigger I2C write and clear flags
            if (all_updated)
            {
                ssd1306_UpdateScreen();           // Trigger I2C write
                clear_all_updated_flags();        // Clear update flags
                refresh_requested = false;        // Clear refresh flag
            }
        }
    }
}

void ancit_oled_handler_force_update(void) {
    // Force immediate update of ALL variables if not busy
    if (!ssd1306_IsBusy()) {
        ancit_oled_update_all_variables();
        ssd1306_UpdateScreen();
        last_refresh_time = ancit_GetTick();
    }
}

bool ancit_oled_handler_is_busy(void) {
    return ssd1306_IsBusy();
}
