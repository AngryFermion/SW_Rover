/*
 * SK_DC.c
 *
 *  Created on: 16-Apr-2026
 *      Author: SasiPrasanthSakhinal
 */

#include "SK_DC.h"



void ancit_smartkit_dc(uint8_t *buffer){

	if(buffer[4] == '2'){
		genx_PWM_LFM_updateDutyCycle(100 - ggenx.PWM);
		genx_PWM_LBM_updateDutyCycle(100);
		genx_PWM_RFM_updateDutyCycle(100 - ggenx.PWM);
		genx_PWM_RBM_updateDutyCycle(100);

	}
	else if(buffer[4] == '1'){
		genx_PWM_LFM_updateDutyCycle(100);
		genx_PWM_LBM_updateDutyCycle(100 - ggenx.PWM);
		genx_PWM_RFM_updateDutyCycle(100);
		genx_PWM_RBM_updateDutyCycle(100 - ggenx.PWM);
	}
	else if(buffer[4] == '3'){
		genx_PWM_LFM_updateDutyCycle(100 - ggenx.PWM);
		genx_PWM_LBM_updateDutyCycle(100);
		genx_PWM_RFM_updateDutyCycle(100);
		genx_PWM_RBM_updateDutyCycle(100 - ggenx.PWM);
	}
	else if(buffer[4] == '4'){
		genx_PWM_LFM_updateDutyCycle(100);
		genx_PWM_LBM_updateDutyCycle(100 - ggenx.PWM);
		genx_PWM_RFM_updateDutyCycle(100 - ggenx.PWM);
		genx_PWM_RBM_updateDutyCycle(100);
	}
	else {

		genx_PWM_LFM_updateDutyCycle(100);
		genx_PWM_LBM_updateDutyCycle(100);
		genx_PWM_RFM_updateDutyCycle(100);
		genx_PWM_RBM_updateDutyCycle(100);

	}
}
