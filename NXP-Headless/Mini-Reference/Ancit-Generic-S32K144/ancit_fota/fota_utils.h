/*
 * fota_utils.h
 *
 *  Created on: 24-Sep-2025
 *      Author: Lenovo
 */

#ifndef FOTA_UTILS_H_
#define FOTA_UTILS_H_
#include "main.h"
#include "S32K144.h"

static uint8_t hexToNibble(char);
void asciiHexToBytes(uint8_t *dst, const char *src, size_t dst_len);
uint32_t asciiHexToUint32(const char *src);
uint8_t asciiHexToByte(const char *src);
void Jump_to_App(uint32_t ADDR_APP);

#endif /* FOTA_UTILS_H_ */
