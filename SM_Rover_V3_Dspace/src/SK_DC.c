/*
 * SK_DC.c
 *
 *  Created on: 16-Apr-2026
 *      Author: SasiPrasanthSakhinal
 */

#include "SK_DC.h"
#include "genx_common.h"
#include "genx_simulink_bridge.h"





void ancit_smartkit_dc(void){
 // 100 is minimum and 0 is maximum PWM

	if (ggenx.Mode == 0){
		//Neutral - Kit Stop Moving
		genx_PWM_LFM_updateDutyCycle(100);
		genx_PWM_LBM_updateDutyCycle(100);
		genx_PWM_RFM_updateDutyCycle(100);
		genx_PWM_RBM_updateDutyCycle(100);
		//display
		ggenx.obj_distance = 0;
		ggenx.PWM_disp = 0;
		ggenx.Vset_disp = 0;
		ggenx.Dmin_disp = 0;
	}

	else if(ggenx.Mode == 2){
		//Reverse - Kit Moves Backward
		genx_PWM_LFM_updateDutyCycle(80);
		genx_PWM_LBM_updateDutyCycle(100);
		genx_PWM_RFM_updateDutyCycle(80);
		genx_PWM_RBM_updateDutyCycle(100);
		//display
		ggenx.obj_distance = 0;
		ggenx.PWM_disp = 20;
		ggenx.Vset_disp = 0;
		ggenx.Dmin_disp = 0;
	}

	else if(ggenx.Mode == 1){
		//Drive mode
		if(((ggenx.VACC == 3) || (ggenx.VACC == 0)) && ggenx.ACC == 0){
			// ACC off state
			genx_PWM_LFM_updateDutyCycle(100);
			genx_PWM_LBM_updateDutyCycle((ggenx.Speed > 100U) ? 0U : (100U - ggenx.Speed));
			//genx_PWM_LBM_updateDutyCycle(100U - ((((ggenx.Speed > 100U) ? 100U : ggenx.Speed) * 30U) / 100U));
			genx_PWM_RFM_updateDutyCycle(100);
			genx_PWM_RBM_updateDutyCycle((ggenx.Speed > 100U) ? 0U : (100U - ggenx.Speed));
			//genx_PWM_RBM_updateDutyCycle(100U - ((((ggenx.Speed > 100U) ? 100U : ggenx.Speed) * 30U) / 100U));

			//display
			ggenx.obj_distance = 0;
			ggenx.PWM_disp = (ggenx.Speed > 100U) ? 100U : (ggenx.Speed);
			ggenx.Vset_disp = 0;
			ggenx.Dmin_disp = 0;
		}
		else if(ggenx.VACC == 0 && ggenx.ACC == 3){
			//Virtual ACC off state
			ANCIT_App_PreStep();
			ACC_step();
			ANCIT_App_PostStep();
			genx_PWM_LFM_updateDutyCycle(100);
			genx_PWM_LBM_updateDutyCycle(100 - (ggenx.PWM/2));
			genx_PWM_RFM_updateDutyCycle(100);
			genx_PWM_RBM_updateDutyCycle(100 - (ggenx.PWM/2));

			//display
			ggenx.obj_distance = ggenx.Ultra_Distance;
			ggenx.PWM_disp = ggenx.PWM;
			ggenx.Vset_disp = ggenx.Vset;
			ggenx.Dmin_disp = ggenx.Dmin;

		}

		else if(ggenx.VACC == 3 && ggenx.ACC == 3){
			//Virtual ACC On state
			ggenx.obj_distance = ggenx.ACC_Obj;
			ggenx.ACC_Obj = (ggenx.ACC_Obj >254U ||  ggenx.ACC_Obj == 0) ? 254U : ggenx.ACC_Obj;

			ANCIT_App_PreStep_Dspace();
			ACC_step();
			ANCIT_App_PostStep();
			genx_PWM_LFM_updateDutyCycle(100);
			genx_PWM_LBM_updateDutyCycle(100 - (ggenx.PWM/2));
			genx_PWM_RFM_updateDutyCycle(100);
			genx_PWM_RBM_updateDutyCycle(100 - (ggenx.PWM/2));

			//display

			ggenx.PWM_disp = ggenx.PWM;
			ggenx.Vset_disp = ggenx.ACC_Vset;
			ggenx.Dmin_disp = ggenx.ACC_Dmin;
		}

		else{
			genx_PWM_LFM_updateDutyCycle(100);
			genx_PWM_LBM_updateDutyCycle((ggenx.Speed > 100U) ? 0U : (100U - ggenx.Speed));
			//genx_PWM_LBM_updateDutyCycle(100U - ((((ggenx.Speed > 100U) ? 100U : ggenx.Speed) * 30U) / 100U));
			genx_PWM_RFM_updateDutyCycle(100);
			genx_PWM_RBM_updateDutyCycle((ggenx.Speed > 100U) ? 0U : (100U - ggenx.Speed));
			//genx_PWM_RBM_updateDutyCycle(100U - ((((ggenx.Speed > 100U) ? 100U : ggenx.Speed) * 30U) / 100U));
			//display
			ggenx.obj_distance = 0;
			ggenx.PWM_disp = (ggenx.Speed > 100U) ? 100U : (ggenx.Speed);
			ggenx.Vset_disp = 0;
			ggenx.Dmin_disp = 0;
		}

	}

	else {
		//Kit Stop Moving
		genx_PWM_LFM_updateDutyCycle(100);
		genx_PWM_LBM_updateDutyCycle(100);
		genx_PWM_RFM_updateDutyCycle(100);
		genx_PWM_RBM_updateDutyCycle(100);

		//display
		ggenx.obj_distance = 0;
		ggenx.PWM_disp = 0;
		ggenx.Vset_disp = 0;
		ggenx.Dmin_disp = 0;

	}
}
