/**
 * Private configuration file for the SSD1306 library.
 * This example is configured for STM32F0, I2C and including all fonts.
 */

#ifndef __SSD1306_CONF_H__
#define __SSD1306_CONF_H__


// Mirror the screen if needed
// #define SSD1306_MIRROR_VERT
// #define SSD1306_MIRROR_HORIZ

// Set inverse color if needed
// # define SSD1306_INVERSE_COLOR

// Default text color configuration
// Uncomment to use black text on white background (inverted)
// Default: White text on black background
// #define SSD1306_TEXT_COLOR_INVERTED

#ifndef SSD1306_TEXT_COLOR_INVERTED
    #define SSD1306_DEFAULT_TEXT_COLOR   White
    #define SSD1306_DEFAULT_BG_COLOR     Black
#else
    #define SSD1306_DEFAULT_TEXT_COLOR   Black
    #define SSD1306_DEFAULT_BG_COLOR     White
#endif

// Set X offset for displays that do not start at column 0
// Testing different values to fix border alignment
#define SSD1306_X_OFFSET 0

// Include only needed fonts
#define SSD1306_INCLUDE_FONT_6x8
#define SSD1306_INCLUDE_FONT_7x10
#define SSD1306_INCLUDE_FONT_11x18
#define SSD1306_INCLUDE_FONT_16x26

#define SSD1306_INCLUDE_FONT_16x24

#define SSD1306_INCLUDE_FONT_16x15

#endif /* __SSD1306_CONF_H__ */
