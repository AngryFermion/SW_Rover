/*
 * ancit_oled_driver.c
 *
 * OLED Device Driver Implementation
 * Based on afiskon/stm32-ssd1306 library, adapted for non-blocking I2C
 *
 * Created on: Jan 24, 2025
 * Author: Claude AI Assistant
 */

#include <ancit_i2c_manager.h>
#include <ancit_oled_driver.h>


#include "ssd1306_fonts.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

/*******************************************************************************
 * Private Variables
 ******************************************************************************/

// Screen buffer (current)
static uint8_t SSD1306_Buffer[SSD1306_BUFFER_SIZE];

// Screen object
static SSD1306_t SSD1306;

// OLED state machine
static struct {
    OLED_State_t currentState;
    uint8_t currentPage;
    bool updatePending;
    uint8_t commandToSend;
} oledMachine;

// Transaction structure for I2C manager
static i2c_transaction_t oledTransaction;

// Buffer for command/data packets
// Need extra 2 bytes for displays that show columns 2-129
static uint8_t oledTxBuffer[SSD1306_WIDTH + 3];  // Control byte + 2 padding + data

/*******************************************************************************
 * Private Function Prototypes
 ******************************************************************************/

static void oled_page_complete_callback(bool success);
static void oled_error_callback(void);
static void oled_submit_page_update(void);
static void oled_send_command_blocking(uint8_t cmd);
static void oled_state_machine(void);

/*******************************************************************************
 * Initialization and State Machine
 ******************************************************************************/

void ssd1306_Init(void) {
    // Initialize state machine
    oledMachine.currentState = OLED_STATE_IDLE;
    oledMachine.currentPage = 0;
    oledMachine.updatePending = false;

    // Wait for display to boot
    OSIF_TimeDelay(100);

    // Init OLED (using blocking commands during init)
    ssd1306_SetDisplayOn(0); // Display off

    oled_send_command_blocking(0x20); // Set Memory Addressing Mode
    oled_send_command_blocking(0x00); // Horizontal Addressing Mode

    oled_send_command_blocking(0xB0); // Set Page Start Address

#ifdef SSD1306_MIRROR_VERT
    oled_send_command_blocking(0xC0); // Mirror vertically
#else
    oled_send_command_blocking(0xC8); // Set COM Output Scan Direction
#endif

    oled_send_command_blocking(0x00 + SSD1306_X_OFFSET_LOWER); // Set low column address
    oled_send_command_blocking(0x10 + SSD1306_X_OFFSET_UPPER); // Set high column address
    oled_send_command_blocking(0x40); // Set start line address

    ssd1306_SetContrast(0xFF);

#ifdef SSD1306_MIRROR_HORIZ
    oled_send_command_blocking(0xA0); // Mirror horizontally
#else
    oled_send_command_blocking(0xA1); // Set segment re-map
#endif

#ifdef SSD1306_INVERSE_COLOR
    oled_send_command_blocking(0xA7); // Set inverse color
#else
    oled_send_command_blocking(0xA6); // Set normal color
#endif

    // Set multiplex ratio
    oled_send_command_blocking(0xA8);
#if (SSD1306_HEIGHT == 32)
    oled_send_command_blocking(0x1F);
#elif (SSD1306_HEIGHT == 64)
    oled_send_command_blocking(0x3F);
#elif (SSD1306_HEIGHT == 128)
    oled_send_command_blocking(0x3F);
#else
#error "Only 32, 64, or 128 lines of height are supported!"
#endif

    oled_send_command_blocking(0xA4); // Output follows RAM content

    oled_send_command_blocking(0xD3); // Set display offset
    oled_send_command_blocking(0x00); // Not offset

    oled_send_command_blocking(0xD5); // Set display clock divide ratio
    oled_send_command_blocking(0xF0); // Set divide ratio

    oled_send_command_blocking(0xD9); // Set pre-charge period
    oled_send_command_blocking(0x22);

    oled_send_command_blocking(0xDA); // Set com pins hardware configuration
#if (SSD1306_HEIGHT == 32)
    oled_send_command_blocking(0x02);
#elif (SSD1306_HEIGHT == 64)
    oled_send_command_blocking(0x12);
#elif (SSD1306_HEIGHT == 128)
    oled_send_command_blocking(0x12);
#endif

    oled_send_command_blocking(0xDB); // Set vcomh
    oled_send_command_blocking(0x20);

    oled_send_command_blocking(0x8D); // Set DC-DC enable
    oled_send_command_blocking(0x14);

    ssd1306_SetDisplayOn(1); // Turn on display

    // Initialize buffer to known state (clear with default background color)
    ssd1306_Clear();
    ssd1306_UpdateScreenBlocking();

    // Set default values
    SSD1306.CurrentX = 0;
    SSD1306.CurrentY = 0;
    SSD1306.Initialized = 1;

    // Note: First screen update will be done by the application
    // The I2C manager must be running in the main loop for updates to work
}

void ssd1306_Main(void) {
    oled_state_machine();
}

void ssd1306_UpdateScreen(void) {
    if (oledMachine.currentState == OLED_STATE_IDLE) {
        // Always update all pages (0 through 7)
        oledMachine.currentPage = 0;
        oledMachine.currentState = OLED_STATE_SUBMIT_PAGE;
        oledMachine.updatePending = true;
    }
}

bool ssd1306_IsBusy(void) {
    return (oledMachine.currentState != OLED_STATE_IDLE);
}

void ssd1306_UpdateScreenBlocking(void) {
    // Initiate the screen update
    ssd1306_UpdateScreen();

    // Wait for the update to complete (blocking)
    // The state machine will return to IDLE when done
    uint32_t timeout = 100;  // 100ms timeout (enough for all 8 pages)
    uint32_t startTime = OSIF_GetMilliseconds();

    while (oledMachine.currentState != OLED_STATE_IDLE) {
        // Run both I2C manager and OLED state machines
        ancit_i2c_manager_main();  // Process I2C transactions
        ssd1306_Main();             // Process OLED updates

        // Check for timeout
        if ((OSIF_GetMilliseconds() - startTime) > timeout) {
            // Timeout - reset state machine
            oledMachine.currentState = OLED_STATE_IDLE;
            oledMachine.updatePending = false;
            break;
        }
    }
}

/*******************************************************************************
 * State Machine Implementation
 ******************************************************************************/

static void oled_state_machine(void) {
    switch (oledMachine.currentState) {
    case OLED_STATE_IDLE:
        // Nothing to do
        break;

    case OLED_STATE_SUBMIT_PAGE:
        oled_submit_page_update();
        break;

    case OLED_STATE_WAIT_PAGE_COMPLETE:
        // Waiting for callback
        break;

    case OLED_STATE_NEXT_PAGE:
        // Move to next page
        oledMachine.currentPage++;

        if (oledMachine.currentPage < (SSD1306_HEIGHT / 8)) {
            // More pages to send
            oledMachine.currentState = OLED_STATE_SUBMIT_PAGE;
        } else {
            // All pages sent
            oledMachine.currentState = OLED_STATE_DONE;
        }
        break;

    case OLED_STATE_DONE:
        oledMachine.updatePending = false;
        oledMachine.currentState = OLED_STATE_IDLE;
        break;

    case OLED_STATE_ERROR:
        oledMachine.updatePending = false;
        oledMachine.currentState = OLED_STATE_IDLE;
        break;

    default:
        oledMachine.currentState = OLED_STATE_ERROR;
        break;
    }
}

static void oled_submit_page_update(void) {
    // Optimize: Combine all commands into fewer transactions
    // Step 0: Send all positioning commands in one transaction (no STOP)
    // Step 1: Send page data with STOP

    oledTransaction.stepCount = 2;  // Reduced from 4 to 2!

    // Step 0: Combined command buffer - all positioning commands
    static uint8_t cmdBuffer[8];
    cmdBuffer[0] = 0x00;  // Control byte (command)
    cmdBuffer[1] = 0xB0 + oledMachine.currentPage;  // Set page address
    cmdBuffer[2] = 0x00;  // Control byte (command)
    cmdBuffer[3] = 0x00 + SSD1306_X_OFFSET_LOWER;   // Set column low
    cmdBuffer[4] = 0x00;  // Control byte (command)
    cmdBuffer[5] = 0x10 + SSD1306_X_OFFSET_UPPER;   // Set column high

    oledTransaction.steps[0].type = I2C_TRANSFER_WRITE;
    oledTransaction.steps[0].buffer = cmdBuffer;
    oledTransaction.steps[0].size = 6;
    oledTransaction.steps[0].sendStop = false;  // Don't stop - continue to data

    // Step 1: Send page data with 2-byte padding for displays showing columns 2-129
    oledTxBuffer[0] = 0x40;  // Control byte (data)
    oledTxBuffer[1] = 0x00;  // Padding byte 1 (column 0 - not visible)
    oledTxBuffer[2] = 0x00;  // Padding byte 2 (column 1 - not visible)
    memcpy(&oledTxBuffer[3],
           &SSD1306_Buffer[SSD1306_WIDTH * oledMachine.currentPage],
           SSD1306_WIDTH);
    oledTransaction.steps[1].type = I2C_TRANSFER_WRITE;
    oledTransaction.steps[1].buffer = oledTxBuffer;
    oledTransaction.steps[1].size = SSD1306_WIDTH + 3;  // Control + 2 padding + 128 data
    oledTransaction.steps[1].sendStop = true;  // Stop after data

    // Submit request
    bool success = ancit_i2c_submit_request(
        I2C_DEVICE_OLED,
        &oledTransaction,
        oled_page_complete_callback,
        oled_error_callback
    );

    if (success) {
        oledMachine.currentState = OLED_STATE_WAIT_PAGE_COMPLETE;
    } else {
        oledMachine.currentState = OLED_STATE_ERROR;
    }
}

static void oled_page_complete_callback(bool success) {
    if (success && oledMachine.currentState == OLED_STATE_WAIT_PAGE_COMPLETE) {
        oledMachine.currentState = OLED_STATE_NEXT_PAGE;
    } else {
        oledMachine.currentState = OLED_STATE_ERROR;
    }
}

static void oled_error_callback(void) {
    oledMachine.currentState = OLED_STATE_ERROR;
}

static void oled_send_command_blocking(uint8_t cmd) {
    uint8_t buffer[2];
    buffer[0] = 0x00;  // Control byte (command)
    buffer[1] = cmd;

    LPI2C_DRV_MasterSetSlaveAddr(INST_LPI2C0, OLED_I2C_ADDR, false);
    LPI2C_DRV_MasterSendDataBlocking(INST_LPI2C0, buffer, 2, true, 100);
}

/*******************************************************************************
 * Drawing Functions (operate on local buffer)
 ******************************************************************************/

void ssd1306_Fill(SSD1306_COLOR color) {
    memset(SSD1306_Buffer, (color == Black) ? 0x00 : 0xFF, sizeof(SSD1306_Buffer));
}

void ssd1306_Clear(void) {
    ssd1306_Fill(SSD1306_DEFAULT_BG_COLOR);
}

void ssd1306_DrawPixel(uint8_t x, uint8_t y, SSD1306_COLOR color) {
    if (x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT) {
        return;
    }

    uint16_t buffer_offset = x + (y / 8) * SSD1306_WIDTH;
    uint8_t bit_mask = 1 << (y % 8);

    if (color == White) {
        SSD1306_Buffer[buffer_offset] |= bit_mask;
    } else {
        SSD1306_Buffer[buffer_offset] &= ~bit_mask;
    }
}


void ssd1306_SetCursor(uint8_t x, uint8_t y) {
    SSD1306.CurrentX = x;
    SSD1306.CurrentY = y;
}

void ssd1306_SetContrast(const uint8_t value) {
    oled_send_command_blocking(0x81);
    oled_send_command_blocking(value);
}

void ssd1306_SetDisplayOn(const uint8_t on) {
    if (on) {
        oled_send_command_blocking(0xAF);
        SSD1306.DisplayOn = 1;
    } else {
        oled_send_command_blocking(0xAE);
        SSD1306.DisplayOn = 0;
    }
}

uint8_t ssd1306_GetDisplayOn(void) {
    return SSD1306.DisplayOn;
}

SSD1306_Error_t ssd1306_FillBuffer(uint8_t* buf, uint32_t len) {
    if (len <= SSD1306_BUFFER_SIZE) {
        memcpy(SSD1306_Buffer, buf, len);
        return SSD1306_OK;
    }
    return SSD1306_ERR;
}

/*******************************************************************************
 * Additional Drawing Functions
 * Copied from old_drivers/OLED/ssd1306.c
 * These functions all operate on SSD1306_Buffer and don't need modification.
 ******************************************************************************/

/*******************************************************************************
 * Helper Functions for Arc Drawing
 ******************************************************************************/

/* Normalize degree to [0;360] */
static uint16_t ssd1306_NormalizeTo0_360(uint16_t par_deg) {
    uint16_t loc_angle;
    if(par_deg <= 360) {
        loc_angle = par_deg;
    } else {
        loc_angle = par_deg % 360;
        loc_angle = (loc_angle ? loc_angle : 360);
    }
    return loc_angle;
}

/* Integer sine approximation using lookup table */
/* Returns value * 128 (fixed point with 7 fractional bits) */
static int16_t ssd1306_sin_int(uint16_t angle) {
    /* Sine lookup table for 0-90 degrees in 1-degree steps */
    /* Values are sin(angle) * 128 */
    static const int16_t sin_table[91] = {
        0, 2, 4, 7, 9, 11, 13, 16, 18, 20,          // 0-9
        22, 24, 27, 29, 31, 33, 35, 37, 40, 42,     // 10-19
        44, 46, 48, 50, 52, 54, 56, 58, 60, 62,     // 20-29
        64, 66, 68, 70, 72, 73, 75, 77, 79, 81,     // 30-39
        82, 84, 86, 87, 89, 91, 92, 94, 95, 97,     // 40-49
        98, 100, 101, 103, 104, 105, 107, 108, 109, 110, // 50-59
        112, 113, 114, 115, 116, 117, 118, 119, 120, 121, // 60-69
        122, 123, 124, 124, 125, 126, 126, 127, 127, 128, // 70-79
        128, 128, 129, 129, 129, 129, 129, 130, 130, 130, // 80-89
        130                                          // 90
    };

    angle = angle % 360;

    if (angle <= 90) {
        return sin_table[angle];
    } else if (angle <= 180) {
        return sin_table[180 - angle];
    } else if (angle <= 270) {
        return -sin_table[angle - 180];
    } else {
        return -sin_table[360 - angle];
    }
}

/* Integer cosine approximation using sine */
static int16_t ssd1306_cos_int(uint16_t angle) {
    return ssd1306_sin_int((angle + 90) % 360);
}

/*******************************************************************************
 * Line and Polyline Drawing
 ******************************************************************************/

/* Draw line by Bresenhem's algorithm */
void ssd1306_DrawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2) {
    int32_t deltaX = abs(x2 - x1);
    int32_t deltaY = abs(y2 - y1);
    int32_t signX = ((x1 < x2) ? 1 : -1);
    int32_t signY = ((y1 < y2) ? 1 : -1);
    int32_t error = deltaX - deltaY;
    int32_t error2;

    ssd1306_DrawPixel(x2, y2, SSD1306_DEFAULT_TEXT_COLOR);

    while((x1 != x2) || (y1 != y2)) {
        ssd1306_DrawPixel(x1, y1, SSD1306_DEFAULT_TEXT_COLOR);
        error2 = error * 2;
        if(error2 > -deltaY) {
            error -= deltaY;
            x1 += signX;
        }

        if(error2 < deltaX) {
            error += deltaX;
            y1 += signY;
        }
    }
    return;
}

/* Draw polyline */
void ssd1306_Polyline(const SSD1306_VERTEX *par_vertex, uint16_t par_size) {
    uint16_t i;
    if(par_vertex == NULL) {
        return;
    }

    for(i = 1; i < par_size; i++) {
        ssd1306_DrawLine(par_vertex[i - 1].x, par_vertex[i - 1].y, par_vertex[i].x, par_vertex[i].y);
    }

    return;
}

/*******************************************************************************
 * Arc Drawing
 ******************************************************************************/

/*
 * DrawArc. Draw angle is beginning from 4 quart of trigonometric circle (3pi/2)
 * start_angle in degree
 * sweep in degree
 */
void ssd1306_DrawArc(uint8_t x, uint8_t y, uint8_t radius, uint16_t start_angle, uint16_t sweep) {
    uint32_t approx_segments;
    uint8_t xp1, xp2;
    uint8_t yp1, yp2;
    uint32_t count;
    uint32_t loc_sweep;
    uint16_t angle1, angle2;

    loc_sweep = ssd1306_NormalizeTo0_360(sweep);

    // Draw arc using integer math (1 degree steps)
    approx_segments = loc_sweep;

    for (count = 0; count < approx_segments; count++) {
        angle1 = ssd1306_NormalizeTo0_360(start_angle + count);
        angle2 = ssd1306_NormalizeTo0_360(start_angle + count + 1);

        // Calculate points using integer sine/cosine (fixed point * 128)
        xp1 = x + ((ssd1306_sin_int(angle1) * radius) >> 7);
        yp1 = y + ((ssd1306_cos_int(angle1) * radius) >> 7);
        xp2 = x + ((ssd1306_sin_int(angle2) * radius) >> 7);
        yp2 = y + ((ssd1306_cos_int(angle2) * radius) >> 7);

        ssd1306_DrawLine(xp1, yp1, xp2, yp2);
    }

    return;
}

/*
 * Draw arc with radius line
 * Angle is beginning from 4 quart of trigonometric circle (3pi/2)
 * start_angle: start angle in degree
 * sweep: finish angle in degree
 */
void ssd1306_DrawArcWithRadiusLine(uint8_t x, uint8_t y, uint8_t radius, uint16_t start_angle, uint16_t sweep) {
    uint32_t approx_segments;
    uint8_t xp1, xp2;
    uint8_t yp1, yp2;
    uint32_t count;
    uint32_t loc_sweep;
    uint16_t angle1, angle2;
    uint8_t first_point_x, first_point_y;
    uint8_t last_point_x, last_point_y;

    loc_sweep = ssd1306_NormalizeTo0_360(sweep);

    // Calculate first point
    angle1 = ssd1306_NormalizeTo0_360(start_angle);
    first_point_x = x + ((ssd1306_sin_int(angle1) * radius) >> 7);
    first_point_y = y + ((ssd1306_cos_int(angle1) * radius) >> 7);

    // Initialize last point to first point (in case sweep is 0)
    last_point_x = first_point_x;
    last_point_y = first_point_y;

    // Draw arc using integer math (1 degree steps)
    approx_segments = loc_sweep;

    for (count = 0; count < approx_segments; count++) {
        angle1 = ssd1306_NormalizeTo0_360(start_angle + count);
        angle2 = ssd1306_NormalizeTo0_360(start_angle + count + 1);

        // Calculate points using integer sine/cosine (fixed point * 128)
        xp1 = x + ((ssd1306_sin_int(angle1) * radius) >> 7);
        yp1 = y + ((ssd1306_cos_int(angle1) * radius) >> 7);
        xp2 = x + ((ssd1306_sin_int(angle2) * radius) >> 7);
        yp2 = y + ((ssd1306_cos_int(angle2) * radius) >> 7);

        ssd1306_DrawLine(xp1, yp1, xp2, yp2);

        // Track last point
        last_point_x = xp2;
        last_point_y = yp2;
    }

    // Draw radius lines
    ssd1306_DrawLine(x, y, first_point_x, first_point_y);
    ssd1306_DrawLine(x, y, last_point_x, last_point_y);

    return;
}

/*******************************************************************************
 * Circle Drawing
 ******************************************************************************/

/* Draw circle by Bresenhem's algorithm */
void ssd1306_DrawCircle(uint8_t par_x, uint8_t par_y, uint8_t par_r) {
    int32_t x = -par_r;
    int32_t y = 0;
    int32_t err = 2 - 2 * par_r;
    int32_t e2;

    if (par_x >= SSD1306_WIDTH || par_y >= SSD1306_HEIGHT) {
        return;
    }

    do {
        ssd1306_DrawPixel(par_x - x, par_y + y, SSD1306_DEFAULT_TEXT_COLOR);
        ssd1306_DrawPixel(par_x + x, par_y + y, SSD1306_DEFAULT_TEXT_COLOR);
        ssd1306_DrawPixel(par_x + x, par_y - y, SSD1306_DEFAULT_TEXT_COLOR);
        ssd1306_DrawPixel(par_x - x, par_y - y, SSD1306_DEFAULT_TEXT_COLOR);
        e2 = err;

        if (e2 <= y) {
            y++;
            err = err + (y * 2 + 1);
            if(-x == y && e2 <= x) {
                e2 = 0;
            }
        }

        if (e2 > x) {
            x++;
            err = err + (x * 2 + 1);
        }
    } while (x <= 0);

    return;
}

/* Draw filled circle. Pixel positions calculated using Bresenham's algorithm */
void ssd1306_FillCircle(uint8_t par_x, uint8_t par_y, uint8_t par_r) {
    int32_t x = -par_r;
    int32_t y = 0;
    int32_t err = 2 - 2 * par_r;
    int32_t e2;

    if (par_x >= SSD1306_WIDTH || par_y >= SSD1306_HEIGHT) {
        return;
    }

    do {
        for (uint8_t _y = (par_y + y); _y >= (par_y - y); _y--) {
            for (uint8_t _x = (par_x - x); _x >= (par_x + x); _x--) {
                ssd1306_DrawPixel(_x, _y, SSD1306_DEFAULT_TEXT_COLOR);
            }
        }

        e2 = err;
        if (e2 <= y) {
            y++;
            err = err + (y * 2 + 1);
            if (-x == y && e2 <= x) {
                e2 = 0;
            }
        }

        if (e2 > x) {
            x++;
            err = err + (x * 2 + 1);
        }
    } while (x <= 0);

    return;
}

/*******************************************************************************
 * Rectangle Drawing
 ******************************************************************************/

/* Draw a rectangle */
void ssd1306_DrawRectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2) {
    ssd1306_DrawLine(x1,y1,x2,y1);
    ssd1306_DrawLine(x2,y1,x2,y2);
    ssd1306_DrawLine(x2,y2,x1,y2);
    ssd1306_DrawLine(x1,y2,x1,y1);

    return;
}

/* Draw a filled rectangle */
void ssd1306_FillRectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2) {
    uint8_t x_start = ((x1<=x2) ? x1 : x2);
    uint8_t x_end   = ((x1<=x2) ? x2 : x1);
    uint8_t y_start = ((y1<=y2) ? y1 : y2);
    uint8_t y_end   = ((y1<=y2) ? y2 : y1);

    for (uint8_t y= y_start; (y<= y_end)&&(y<SSD1306_HEIGHT); y++) {
        for (uint8_t x= x_start; (x<= x_end)&&(x<SSD1306_WIDTH); x++) {
            ssd1306_DrawPixel(x, y, SSD1306_DEFAULT_TEXT_COLOR);
        }
    }
    return;
}

SSD1306_Error_t ssd1306_InvertRectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2) {
  if ((x2 >= SSD1306_WIDTH) || (y2 >= SSD1306_HEIGHT)) {
    return SSD1306_ERR;
  }
  if ((x1 > x2) || (y1 > y2)) {
    return SSD1306_ERR;
  }
  uint32_t i;
  if ((y1 / 8) != (y2 / 8)) {
    /* if rectangle doesn't lie on one 8px row */
    for (uint32_t x = x1; x <= x2; x++) {
      i = x + (y1 / 8) * SSD1306_WIDTH;
      SSD1306_Buffer[i] ^= 0xFF << (y1 % 8);
      i += SSD1306_WIDTH;
      for (; i < x + (y2 / 8) * SSD1306_WIDTH; i += SSD1306_WIDTH) {
        SSD1306_Buffer[i] ^= 0xFF;
      }
      SSD1306_Buffer[i] ^= 0xFF >> (7 - (y2 % 8));
    }
  } else {
    /* if rectangle lies on one 8px row */
    const uint8_t mask = (0xFF << (y1 % 8)) & (0xFF >> (7 - (y2 % 8)));
    for (i = x1 + (y1 / 8) * SSD1306_WIDTH;
         i <= (uint32_t)x2 + (y2 / 8) * SSD1306_WIDTH; i++) {
      SSD1306_Buffer[i] ^= mask;
    }
  }
  return SSD1306_OK;
}

/*******************************************************************************
 * Bitmap Drawing
 ******************************************************************************/

/* Draw a bitmap */
void ssd1306_DrawBitmap(uint8_t x, uint8_t y, const unsigned char* bitmap, uint8_t w, uint8_t h) {
    int16_t byteWidth = (w + 7) / 8; // Bitmap scanline pad = whole byte
    uint8_t byte = 0;

    if (x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT) {
        return;
    }

    for (uint8_t j = 0; j < h; j++, y++) {
        for (uint8_t i = 0; i < w; i++) {
            if (i & 7) {
                byte <<= 1;
            } else {
                byte = (*(const unsigned char *)(&bitmap[j * byteWidth + i / 8]));
            }

            if (byte & 0x80) {
                ssd1306_DrawPixel(x + i, y, SSD1306_DEFAULT_TEXT_COLOR);
            }
        }
    }
    return;
}

/*******************************************************************************
 * Simple Blocking Drawing Functions
 ******************************************************************************/

char ssd1306_WriteChar(char ch, SSD1306_Font_t Font) {
    // Declare all variables at function start for better register allocation
    uint32_t i, b, j;
    uint8_t char_width;
    uint16_t char_offset;
    uint8_t y;
    uint16_t row_offset;
    uint8_t bit_mask;
    uint16_t mask;
#if !SSD1306_SKIP_BACKGROUND_PIXELS
    SSD1306_COLOR color = SSD1306_DEFAULT_TEXT_COLOR;
#endif

    // Validate character
    if (ch < 32 || ch > 126)
        return 0;

    // Get character width
    char_width = Font.char_width ? Font.char_width[ch - 32] : Font.width;

    // Check bounds
    if (SSD1306_WIDTH < (SSD1306.CurrentX + char_width) ||
        SSD1306_HEIGHT < (SSD1306.CurrentY + Font.height)) {
        return 0;
    }

    // Calculate character offset in font data
    char_offset = (ch - 32) * Font.height;
    
#if SSD1306_SKIP_BACKGROUND_PIXELS
    // Fast mode: Draw both foreground and background (white text on black)
    for (i = 0; i < Font.height; i++) {
        b = Font.data[char_offset + i];
        y = SSD1306.CurrentY + i;
        row_offset = ((y >> 3) << 7) + SSD1306.CurrentX;
        bit_mask = 1 << (y & 0x07);
        mask = 0x8000;

        for (j = 0; j < char_width; j++) {
            if (b & mask) {
                SSD1306_Buffer[row_offset + j] |= bit_mask;  // Set foreground
            } else {
                SSD1306_Buffer[row_offset + j] &= ~bit_mask; // Clear background
            }
            mask >>= 1;
        }
    }
#else
    // Full mode: Reuse pre-declared variables with color support
    for (i = 0; i < Font.height; i++) {
        b = Font.data[char_offset + i];
        y = SSD1306.CurrentY + i;
        row_offset = ((y >> 3) << 7) + SSD1306.CurrentX;
        bit_mask = 1 << (y & 0x07);
        mask = 0x8000;

        for (j = 0; j < char_width; j++) {
            if (b & mask) {
                if (color == White) {
                    SSD1306_Buffer[row_offset + j] |= bit_mask;
                } else {
                    SSD1306_Buffer[row_offset + j] &= ~bit_mask;
                }
            } else {
                if (color == White) {
                    SSD1306_Buffer[row_offset + j] &= ~bit_mask;
                } else {
                    SSD1306_Buffer[row_offset + j] |= bit_mask;
                }
            }
            mask >>= 1;
        }
    }
#endif

    // Update cursor
    SSD1306.CurrentX += char_width;
    return ch;
}

uint8_t ssd1306_WriteString(const char* str, SSD1306_Font_t Font, uint8_t x, uint8_t y) {
    uint8_t count = 0;

    if (str == NULL) return 0;

    // Set cursor position
    SSD1306.CurrentX = x;
    SSD1306.CurrentY = y;

    // Draw each character (~200 µs for 13 chars with Font_11x18)
    while (*str) {
        if (ssd1306_WriteChar(*str, Font) != *str) {
            return count;  // Failed to write character (out of bounds)
        }
        str++;
        count++;
    }

    return count;
}

/*******************************************************************************
 * End of OLED Driver Implementation
 ******************************************************************************/

