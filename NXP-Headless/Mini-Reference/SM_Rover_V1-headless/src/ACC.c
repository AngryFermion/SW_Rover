/*
 * ACC.c
 *
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * Code generation for model "ACC".
 *
 * Model version              : 1.15
 * Simulink Coder version : 24.2 (R2024b) 21-Jun-2024
 * C source code generated on : Fri May  8 11:57:33 2026
 *
 * Target selection: grt.tlc
 * Note: GRT includes extra infrastructure and instrumentation for prototyping
 * Embedded hardware selection: Intel->x86-64 (Windows64)
 * Code generation objectives: Unspecified
 * Validation result: Not run
 */

#include "ACC.h"
#include "rtwtypes.h"
#include <string.h>
#include "ACC_private.h"
#define ACC_period                     (0.001)

/* Block states (default storage) */
DW_ACC_T ACC_DW;

/* External inputs (root inport signals with default storage) */
ExtU_ACC_T ACC_U;

/* External outputs (root outports fed by signals with default storage) */
ExtY_ACC_T ACC_Y;

/* Real-time model */
static RT_MODEL_ACC_T ACC_M_;
RT_MODEL_ACC_T *const ACC_M = &ACC_M_;

/* Model step function */
void ACC_step(void)
{
  real_T rtb_Gain5;
  real_T rtb_d_desired;
  real_T rtb_d_filtered;
  real_T rtb_d_prev;

  /* Gain: '<Root>/Gain1' incorporates:
   *  UnitDelay: '<Root>/Unit Delay'
   */
  rtb_d_prev = ACC_P.Gain1_Gain * ACC_DW.UnitDelay_DSTATE;

  /* Sum: '<Root>/Sum' incorporates:
   *  Gain: '<Root>/Gain'
   *  Inport: '<Root>/Distance'
   */
  rtb_d_filtered = ACC_P.Gain_Gain * ACC_U.Distance + rtb_d_prev;

  /* Gain: '<Root>/Gain5' incorporates:
   *  UnitDelay: '<Root>/Unit Delay1'
   */
  rtb_Gain5 = ACC_P.Gain5_Gain * ACC_DW.UnitDelay1_DSTATE;

  /* Sum: '<Root>/Sum1' incorporates:
   *  Gain: '<Root>/Tg'
   *  Inport: '<Root>/D_min'
   */
  rtb_d_desired = ACC_P.Tg_Gain * rtb_Gain5 + ACC_U.D_min;

  /* Switch: '<Root>/Switch' incorporates:
   *  Gain: '<Root>/Gain2'
   *  Gain: '<Root>/Kd_d'
   *  Gain: '<Root>/Kp_d'
   *  Gain: '<Root>/Kp_v'
   *  Gain: '<Root>/Ts'
   *  Inport: '<Root>/V_set'
   *  RelationalOperator: '<Root>/Relational Operator'
   *  Sum: '<Root>/Subtract'
   *  Sum: '<Root>/Sum2'
   *  Sum: '<Root>/Sum3'
   *  Sum: '<Root>/Sum4'
   *  Sum: '<Root>/err_d'
   *  Sum: '<Root>/err_v'
   */
  if (rtb_d_filtered <= rtb_d_desired) {
    rtb_d_prev = ((rtb_d_prev - rtb_d_filtered) * ACC_P.Ts_Gain *
                  ACC_P.Kd_d_Gain + (rtb_d_desired - rtb_d_filtered) *
                  ACC_P.Kp_d_Gain) + rtb_Gain5;
  } else {
    rtb_d_prev = (ACC_P.Gain2_Gain * ACC_U.V_set - rtb_Gain5) * ACC_P.Kp_v_Gain
      + rtb_Gain5;
  }

  /* End of Switch: '<Root>/Switch' */

  /* RateLimiter: '<Root>/Rate Limiter' */
  rtb_d_filtered = rtb_d_prev - ACC_DW.PrevY;
  if (rtb_d_filtered > ACC_P.RateLimiter_RisingLim * ACC_period) {
    rtb_d_prev = ACC_P.RateLimiter_RisingLim * ACC_period + ACC_DW.PrevY;
  } else if (rtb_d_filtered < ACC_P.RateLimiter_FallingLim * ACC_period) {
    rtb_d_prev = ACC_P.RateLimiter_FallingLim * ACC_period + ACC_DW.PrevY;
  }

  ACC_DW.PrevY = rtb_d_prev;

  /* End of RateLimiter: '<Root>/Rate Limiter' */

  /* Saturate: '<Root>/Saturation' */
  if (rtb_d_prev > ACC_P.Saturation_UpperSat) {
    rtb_d_prev = ACC_P.Saturation_UpperSat;
  } else if (rtb_d_prev < ACC_P.Saturation_LowerSat) {
    rtb_d_prev = ACC_P.Saturation_LowerSat;
  }

  /* End of Saturate: '<Root>/Saturation' */

  /* Outport: '<Root>/PWM' incorporates:
   *  Gain: '<Root>/Gain4'
   */
  ACC_Y.PWM = ACC_P.Gain4_Gain * rtb_d_prev;

  /* Update for UnitDelay: '<Root>/Unit Delay' incorporates:
   *  Inport: '<Root>/Distance'
   */
  ACC_DW.UnitDelay_DSTATE = ACC_U.Distance;

  /* Update for UnitDelay: '<Root>/Unit Delay1' */
  ACC_DW.UnitDelay1_DSTATE = rtb_d_prev;

  /* Matfile logging */
  rt_UpdateTXYLogVars(ACC_M->rtwLogInfo, (&ACC_M->Timing.taskTime0));

  /* signal main to stop simulation */
  {                                    /* Sample time: [0.2s, 0.0s] */
    if ((rtmGetTFinal(ACC_M)!=-1) &&
        !((rtmGetTFinal(ACC_M)-ACC_M->Timing.taskTime0) >
          ACC_M->Timing.taskTime0 * (DBL_EPSILON))) {
      rtmSetErrorStatus(ACC_M, "Simulation finished");
    }
  }

  /* Update absolute time for base rate */
  /* The "clockTick0" counts the number of times the code of this task has
   * been executed. The absolute time is the multiplication of "clockTick0"
   * and "Timing.stepSize0". Size of "clockTick0" ensures timer will not
   * overflow during the application lifespan selected.
   * Timer of this task consists of two 32 bit unsigned integers.
   * The two integers represent the low bits Timing.clockTick0 and the high bits
   * Timing.clockTickH0. When the low bit overflows to 0, the high bits increment.
   */
  if (!(++ACC_M->Timing.clockTick0)) {
    ++ACC_M->Timing.clockTickH0;
  }

  ACC_M->Timing.taskTime0 = ACC_M->Timing.clockTick0 * ACC_M->Timing.stepSize0 +
    ACC_M->Timing.clockTickH0 * ACC_M->Timing.stepSize0 * 4294967296.0;
}

/* Model initialize function */
void ACC_initialize(void)
{
  /* Registration code */

  /* initialize real-time model */
  (void) memset((void *)ACC_M, 0,
                sizeof(RT_MODEL_ACC_T));
  rtmSetTFinal(ACC_M, -1);
  ACC_M->Timing.stepSize0 = 0.2;

  /* Setup for data logging */
  {
    static RTWLogInfo rt_DataLoggingInfo;
    rt_DataLoggingInfo.loggingInterval = (NULL);
    ACC_M->rtwLogInfo = &rt_DataLoggingInfo;
  }

  /* Setup for data logging */
  {
    rtliSetLogXSignalInfo(ACC_M->rtwLogInfo, (NULL));
    rtliSetLogXSignalPtrs(ACC_M->rtwLogInfo, (NULL));
    rtliSetLogT(ACC_M->rtwLogInfo, "tout");
    rtliSetLogX(ACC_M->rtwLogInfo, "");
    rtliSetLogXFinal(ACC_M->rtwLogInfo, "");
    rtliSetLogVarNameModifier(ACC_M->rtwLogInfo, "rt_");
    rtliSetLogFormat(ACC_M->rtwLogInfo, 4);
    rtliSetLogMaxRows(ACC_M->rtwLogInfo, 0);
    rtliSetLogDecimation(ACC_M->rtwLogInfo, 1);
    rtliSetLogY(ACC_M->rtwLogInfo, "");
    rtliSetLogYSignalInfo(ACC_M->rtwLogInfo, (NULL));
    rtliSetLogYSignalPtrs(ACC_M->rtwLogInfo, (NULL));
  }

  /* states (dwork) */
  (void) memset((void *)&ACC_DW, 0,
                sizeof(DW_ACC_T));

  /* external inputs */
  (void)memset(&ACC_U, 0, sizeof(ExtU_ACC_T));

  /* external outputs */
  ACC_Y.PWM = 0.0;

  /* Matfile logging */
  rt_StartDataLoggingWithStartTime(ACC_M->rtwLogInfo, 0.0, rtmGetTFinal(ACC_M),
    ACC_M->Timing.stepSize0, (&rtmGetErrorStatus(ACC_M)));

  /* InitializeConditions for UnitDelay: '<Root>/Unit Delay' */
  ACC_DW.UnitDelay_DSTATE = ACC_P.UnitDelay_InitialCondition;

  /* InitializeConditions for UnitDelay: '<Root>/Unit Delay1' */
  ACC_DW.UnitDelay1_DSTATE = ACC_P.UnitDelay1_InitialCondition;

  /* InitializeConditions for RateLimiter: '<Root>/Rate Limiter' */
  ACC_DW.PrevY = ACC_P.RateLimiter_IC;
}

/* Model terminate function */
void ACC_terminate(void)
{
  /* (no terminate code required) */
}
