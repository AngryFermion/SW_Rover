/*
 * fota_utils.c
 *
 *  Created on: 24-Sep-2025
 *      Author: Lenovo
 */

#include "fota_utils.h"

typedef void(*jump_to_app)(uint32);
jump_to_app      *app = NULL;

static uint8_t hexToNibble(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return 0;
}

// Convert 256 ASCII hex chars -> 128 raw bytes
void asciiHexToBytes(uint8_t *dst, const char *src, size_t dst_len) {
    for (size_t i = 0; i < dst_len; i++) {
        uint8_t high = hexToNibble(src[i * 2]);
        uint8_t low  = hexToNibble(src[i * 2 + 1]);
        dst[i] = (high << 4) | low;
    }
}

uint32_t asciiHexToUint32(const char *src) {
    uint32_t value = 0;
    for (int i = 0; i < 8; i++) {
        char c = src[i];
        uint8_t nibble;

        if (c >= '0' && c <= '9') nibble = c - '0';
        else if (c >= 'A' && c <= 'F') nibble = c - 'A' + 10;
        else if (c >= 'a' && c <= 'f') nibble = c - 'a' + 10;
        else nibble = 0;

        value = (value << 4) | nibble;
    }
    return value;
}

uint8_t asciiHexToByte(const char *src) {
    uint8_t high, low;

    if (src[0] >= '0' && src[0] <= '9') high = src[0] - '0';
    else if (src[0] >= 'A' && src[0] <= 'F') high = src[0] - 'A' + 10;
    else if (src[0] >= 'a' && src[0] <= 'f') high = src[0] - 'a' + 10;

    if (src[1] >= '0' && src[1] <= '9') low = src[1] - '0';
    else if (src[1] >= 'A' && src[1] <= 'F') low = src[1] - 'A' + 10;
    else if (src[1] >= 'a' && src[1] <= 'f') low = src[1] - 'a' + 10;

    return (high << 4) | low;
}



void Jump_to_App(uint32_t ADDR_APP)
{
	 //disable the interrupts
	 __asm("cpsid i");

	 app = (uint32_t )ADDR_APP +4;
	 S32_SCB->VTOR = (uint32_t )ADDR_APP;

	 __asm("msr msp,r0");
	 (*app)();
}
