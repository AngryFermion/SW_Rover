/*
 * ancit_dwin.h
 *
 *  Created on: 16-Jun-2024
 *      Author: Narayan
 */

#ifndef ANCIT_DWIN_H_
#define ANCIT_DWIN_H_
#include "genx_config.h"

#ifdef DWIN_DISPLAY_CONFIGURED

//UART packet tx types (FIXED)
#define DWIN_PKT_TX_TYPE_DISABLED		0
#define DWIN_PKT_TX_TYPE_ON_EVENT		1
#define DWIN_PKT_TX_TYPE_CYCLIC			2

typedef enum {
    VALUE_INT16,
    VALUE_INT32,
    VALUE_UINT16,
    VALUE_UINT32,
    VALUE_FLOAT,
    VALUE_DOUBLE
} value_type_t;

typedef union {
    int16_t int16_value;
    int32_t int32_value;
    uint16_t uint16_value;
    uint32_t uint32_value;
    float float_value;
    double double_value;
} value_union_t;

typedef struct {
    uint16_t vp_addr;
    value_type_t type;  // Indicates the type of the value in the union
    bool init;
    uint8_t tx_type;
    value_union_t value;
    bool value_updated;
    bool tx_triggered;
    bool rx_triggered;
} dwin_mem_items_t;

void ancit_dwin_set_value_by_vp_addr(uint16_t vp_addr, double value);
double ancit_dwin_get_value_by_vp_addr(uint16_t vp_addr);
int8_t find_index_by_vp_addr(uint16_t vp_addr);

#endif //DWIN_DISPLAY_CONFIGURED
#endif /* ANCIT_DWIN_H_ */
