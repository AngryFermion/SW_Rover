/*
 * ancit_dwin.c
 *
 *  Created on: 16-Jun-2024
 *      Author: Narayan
 */
#include "ancit_dwin.h"

#ifdef DWIN_DISPLAY_CONFIGURED
#include "../../src/genx_dwin.h"

extern dwin_mem_items_t dwin_mem[DWIN_VP_MAX_ITEMS];

/**
 * Finds the index of a dwin_vp_items_t item in the dwin_vp array based on its vp_addr.
 *
 * @param vp_addr The vp_addr to search for.
 * @return Index of the item with the specified vp_addr, or -1 if not found.
 */
int8_t find_index_by_vp_addr(uint16_t vp_addr) {
	for (int i = 0; i < DWIN_VP_MAX_ITEMS; i++) {
		if (dwin_mem[i].vp_addr == vp_addr) {
			return i;
		}
	}
	return -1; // Return -1 if vp_addr is not found in the array
}

void ancit_dwin_set_value_by_vp_addr(uint16_t vp_addr, double value) {
	int8_t index = find_index_by_vp_addr(vp_addr);
	if (index == -1) {
		//vp_addr not found
		return;
	}

	// Now directly use the index to access the item and set the value
	switch (dwin_mem[index].type) {
	case VALUE_INT16:
		dwin_mem[index].value.int16_value = (int16_t) value;
		break;
	case VALUE_INT32:
		dwin_mem[index].value.int32_value = (int32_t) value;
		break;
	case VALUE_UINT16:
		dwin_mem[index].value.uint16_value = (uint16_t) value;
		break;
	case VALUE_UINT32:
		dwin_mem[index].value.uint32_value = (uint32_t) value;
		break;
	case VALUE_FLOAT:
		dwin_mem[index].value.float_value = (float) value;
		break;
	case VALUE_DOUBLE:
		dwin_mem[index].value.double_value = value;
		break;
	default:
		dwin_mem[index].value.int16_value = (int16_t) value;
		break;
	}
	dwin_mem[index].value_updated = true;
}

double ancit_dwin_get_value_by_vp_addr(uint16_t vp_addr) {
	int8_t index = find_index_by_vp_addr(vp_addr);
	if (index == -1) {
		return 0.0; // Returning 0.0 on not found, could also use NaN or another error indicator
	}

	switch (dwin_mem[index].type) {
	case VALUE_INT16:
		return dwin_mem[index].value.int16_value;  // Promoted to double
	case VALUE_INT32:
		return dwin_mem[index].value.int32_value;  // Promoted to double
	case VALUE_UINT16:
		return dwin_mem[index].value.uint16_value;  // Promoted to double
	case VALUE_UINT32:
		return dwin_mem[index].value.uint32_value;  // Promoted to double
	case VALUE_FLOAT:
		return dwin_mem[index].value.float_value;  // Promoted to double
	case VALUE_DOUBLE:
		return dwin_mem[index].value.double_value;
	default:
		return 0.0;  // Handle default case
	}
}

#endif //DWIN_DISPLAY_CONFIGURED
