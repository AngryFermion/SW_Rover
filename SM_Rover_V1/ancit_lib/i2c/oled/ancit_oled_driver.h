/*
 * ancit_oled_driver.h
 *
 * OLED Device Driver - Non-blocking I2C OLED (SSD1306) driver
 * Uses base I2C manager for all physical I2C operations
 *
 * Created on: Jan 24, 2025
 * Author: Claude AI Assistant
 * Based on: afiskon/stm32-ssd1306 library
 */

#ifndef ANCIT_OLED_DRIVER_H_
#define ANCIT_OLED_DRIVER_H_

#include <ssd1306_conf.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "_ansi.h"
#include "sdk_project_config.h"
#include "genx_config.h"

_BEGIN_STD_C


/*******************************************************************************
 * Configuration
 ******************************************************************************/

#ifdef SSD1306_X_OFFSET
#define SSD1306_X_OFFSET_LOWER (SSD1306_X_OFFSET & 0x0F)
#define SSD1306_X_OFFSET_UPPER ((SSD1306_X_OFFSET >> 4) & 0x07)
#else
#define SSD1306_X_OFFSET_LOWER 0
#define SSD1306_X_OFFSET_UPPER 0
#endif

#ifndef SSD1306_HEIGHT
#define SSD1306_HEIGHT          64
#endif

#ifndef SSD1306_WIDTH
#define SSD1306_WIDTH           128
#endif

#ifndef SSD1306_BUFFER_SIZE
#define SSD1306_BUFFER_SIZE   (SSD1306_WIDTH * SSD1306_HEIGHT / 8)
#endif

// Drawing mode: Now draws both foreground and background pixels
// Set to 1 for white-only text (ignores color parameter, slightly faster)// Set to 0 for full color support (respects color parameter for black/white text)
#ifndef SSD1306_SKIP_BACKGROUND_PIXELS
#define SSD1306_SKIP_BACKGROUND_PIXELS  1
#endif
/*******************************************************************************
 * Type Definitions
 ******************************************************************************/

// Screen colors
typedef enum {
    Black = 0x00, // Black color, no pixel
    White = 0x01  // Pixel is set
} SSD1306_COLOR;

// Error codes
typedef enum {
    SSD1306_OK = 0x00,
    SSD1306_ERR = 0x01
} SSD1306_Error_t;

// SSD1306 context
typedef struct {
    uint16_t CurrentX;
    uint16_t CurrentY;
    uint8_t Initialized;
    uint8_t DisplayOn;
} SSD1306_t;

// Vertex for drawing
typedef struct {
    uint8_t x;
    uint8_t y;
} SSD1306_VERTEX;

// Font structure
typedef struct {
    const uint8_t width;
    const uint8_t height;
    const uint16_t *const data;
    const uint8_t *const char_width;
} SSD1306_Font_t;

// OLED state machine states
typedef enum {
    OLED_STATE_IDLE,
    OLED_STATE_INIT,
    OLED_STATE_UPDATE_INIT,
    OLED_STATE_SUBMIT_PAGE,
    OLED_STATE_WAIT_PAGE_COMPLETE,
    OLED_STATE_NEXT_PAGE,
    OLED_STATE_SEND_COMMAND,
    OLED_STATE_WAIT_COMMAND_COMPLETE,
    OLED_STATE_DONE,
    OLED_STATE_ERROR
} OLED_State_t;

/*******************************************************************************
 * Public API
 ******************************************************************************/

/**
 * @brief Initialize OLED driver
 *
 * Performs blocking initialization sequence.
 * Must be called once during system initialization.
 */
void ssd1306_Init(void);

/**
 * @brief Run OLED state machine
 *
 * Must be called periodically from main loop.
 */
void ssd1306_Main(void);

/**
 * @brief Fill entire screen with color
 *
 * @param color Color to fill (Black or White)
 */
void ssd1306_Fill(SSD1306_COLOR color);

/**
 * @brief Clear screen with default background color
 *
 * Uses SSD1306_DEFAULT_BG_COLOR from configuration.
 * Use this instead of ssd1306_Fill(Black) for automatic inversion support.
 */
void ssd1306_Clear(void);

/**
 * @brief Update screen (non-blocking)
 *
 * Initiates non-blocking screen update.
 * Sends all 8 pages to the display.
 */
void ssd1306_UpdateScreen(void);

/**
 * @brief Check if OLED is busy
 *
 * Returns true if the OLED is currently processing an update.
 * Use this before calling drawing functions or initiating new updates.
 *
 * @return true if busy, false if idle
 */
bool ssd1306_IsBusy(void);

/**
 * @brief Update screen (BLOCKING)
 *
 * Updates the screen and waits until the I2C transfer is complete.
 * Use this during initialization or when you need the display to be
 * fully updated before continuing.
 *
 * @warning This function blocks for up to 100ms while sending data via I2C
 */
void ssd1306_UpdateScreenBlocking(void);

/**
 * @brief Draw a pixel (IMMEDIATE - updates buffer directly)
 *
 * This function is FAST and executes immediately.
 * Use for simple pixel operations.
 *
 * @param x X coordinate
 * @param y Y coordinate
 * @param color Color (Black or White) - for custom use
 */
void ssd1306_DrawPixel(uint8_t x, uint8_t y, SSD1306_COLOR color);

/**
 * @brief Set cursor position
 *
 * @param x X coordinate
 * @param y Y coordinate
 */
void ssd1306_SetCursor(uint8_t x, uint8_t y);

/*******************************************************************************
 * Primary Drawing API - Uses Default Color from Configuration
 ******************************************************************************/

/**
 * @brief Draw a line
 */
void ssd1306_DrawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);

/**
 * @brief Draw an arc
 * @param x Center X coordinate
 * @param y Center Y coordinate
 * @param radius Arc radius
 * @param start_angle Starting angle in degrees
 * @param sweep Sweep angle in degrees
 */
void ssd1306_DrawArc(uint8_t x, uint8_t y, uint8_t radius, uint16_t start_angle, uint16_t sweep);

/**
 * @brief Draw an arc with radius lines
 */
void ssd1306_DrawArcWithRadiusLine(uint8_t x, uint8_t y, uint8_t radius, uint16_t start_angle, uint16_t sweep);

/**
 * @brief Draw a circle outline
 */
void ssd1306_DrawCircle(uint8_t x, uint8_t y, uint8_t r);

/**
 * @brief Draw a filled circle
 */
void ssd1306_FillCircle(uint8_t x, uint8_t y, uint8_t r);

/**
 * @brief Draw a polyline (connected line segments)
 */
void ssd1306_Polyline(const SSD1306_VERTEX *vertices, uint16_t count);

/**
 * @brief Draw a rectangle outline
 */
void ssd1306_DrawRectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);

/**
 * @brief Draw a filled rectangle
 */
void ssd1306_FillRectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);

/**
 * @brief Invert rectangle (toggles pixels)
 */
SSD1306_Error_t ssd1306_InvertRectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);

/**
 * @brief Draw a bitmap
 */
void ssd1306_DrawBitmap(uint8_t x, uint8_t y, const unsigned char* bitmap, uint8_t w, uint8_t h);

/**
 * @brief Set contrast
 *
 * @param value Contrast value (0-255)
 */
void ssd1306_SetContrast(const uint8_t value);

/**
 * @brief Set display on/off
 *
 * @param on 0 for OFF, any for ON
 */
void ssd1306_SetDisplayOn(const uint8_t on);

/**
 * @brief Get display on state
 *
 * @return 0: OFF, 1: ON
 */
uint8_t ssd1306_GetDisplayOn(void);

/**
 * @brief Fill buffer
 */
SSD1306_Error_t ssd1306_FillBuffer(uint8_t* buf, uint32_t len);

/*******************************************************************************
 * Text Drawing API - Uses Default Color from Configuration
 ******************************************************************************/

/**
 * @brief Write a character at current cursor position
 *
 * Draws a character using the specified font.
 * Updates the cursor position automatically.
 *
 * @param ch Character to write
 * @param Font Font to use
 * @return Character written (or 0 if failed)
 *
 * @note This function executes immediately (~100 µs for Font_11x18)
 */
char ssd1306_WriteChar(char ch, SSD1306_Font_t Font);

/**
 * @brief Write a string at specified position
 *
 * Draws a string using the specified font.
 * Simple blocking call - executes immediately.
 *
 * @param str String to write
 * @param Font Font to use
 * @param x X position
 * @param y Y position
 * @return Number of characters written
 *
 * @note This function executes immediately (~200 µs for 13 chars with Font_11x18)
 * @note Uses SSD1306_DEFAULT_TEXT_COLOR from ssd1306_conf.h
 *
 * Example:
 *   ssd1306_WriteString("Count: 123", Font_11x18, 5, 45);
 */
uint8_t ssd1306_WriteString(const char* str, SSD1306_Font_t Font, uint8_t x, uint8_t y);

_END_STD_C

#endif // ANCIT_OLED_DRIVER_H_
