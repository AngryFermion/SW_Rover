/*
 * ACC.h
 *
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * Code generation for model "ACC".
 *
 * Model version              : 1.22
 * Simulink Coder version : 24.2 (R2024b) 21-Jun-2024
 * C source code generated on : Mon May 11 13:30:22 2026
 *
 * Target selection: grt.tlc
 * Note: GRT includes extra infrastructure and instrumentation for prototyping
 * Embedded hardware selection: Intel->x86-64 (Windows64)
 * Code generation objectives: Unspecified
 * Validation result: Not run
 */

#ifndef ACC_h_
#define ACC_h_
#ifndef ACC_COMMON_INCLUDES_
#define ACC_COMMON_INCLUDES_
#include "rtwtypes.h"
#include "rtw_continuous.h"
#include "rtw_solver.h"
#include "rt_logging.h"
#include "rt_nonfinite.h"
#include "math.h"
#endif                                 /* ACC_COMMON_INCLUDES_ */

#include "ACC_types.h"
#include <float.h>
#include <string.h>
#include <stddef.h>

/* Macros for accessing real-time model data structure */
#ifndef rtmGetFinalTime
#define rtmGetFinalTime(rtm)           ((rtm)->Timing.tFinal)
#endif

#ifndef rtmGetRTWLogInfo
#define rtmGetRTWLogInfo(rtm)          ((rtm)->rtwLogInfo)
#endif

#ifndef rtmGetErrorStatus
#define rtmGetErrorStatus(rtm)         ((rtm)->errorStatus)
#endif

#ifndef rtmSetErrorStatus
#define rtmSetErrorStatus(rtm, val)    ((rtm)->errorStatus = (val))
#endif

#ifndef rtmGetStopRequested
#define rtmGetStopRequested(rtm)       ((rtm)->Timing.stopRequestedFlag)
#endif

#ifndef rtmSetStopRequested
#define rtmSetStopRequested(rtm, val)  ((rtm)->Timing.stopRequestedFlag = (val))
#endif

#ifndef rtmGetStopRequestedPtr
#define rtmGetStopRequestedPtr(rtm)    (&((rtm)->Timing.stopRequestedFlag))
#endif

#ifndef rtmGetT
#define rtmGetT(rtm)                   ((rtm)->Timing.taskTime0)
#endif

#ifndef rtmGetTFinal
#define rtmGetTFinal(rtm)              ((rtm)->Timing.tFinal)
#endif

#ifndef rtmGetTPtr
#define rtmGetTPtr(rtm)                (&(rtm)->Timing.taskTime0)
#endif

/* Block states (default storage) for system '<Root>' */
typedef struct {
  real_T UnitDelay_DSTATE;             /* '<Root>/Unit Delay' */
  real_T UnitDelay1_DSTATE;            /* '<Root>/Unit Delay1' */
  real_T PrevY;                        /* '<Root>/Rate Limiter' */
} DW_ACC_T;

/* External inputs (root inport signals with default storage) */
typedef struct {
  real_T Distance;                     /* '<Root>/Distance' */
  real_T V_set;                        /* '<Root>/V_set' */
  real_T D_min;                        /* '<Root>/D_min' */
} ExtU_ACC_T;

/* External outputs (root outports fed by signals with default storage) */
typedef struct {
  real_T PWM;                          /* '<Root>/PWM' */
} ExtY_ACC_T;

/* Parameters (default storage) */
struct P_ACC_T_ {
  real_T Kp_d_Gain;                    /* Expression: 0.000005
                                        * Referenced by: '<Root>/Kp_d'
                                        */
  real_T Ts_Gain;                      /* Expression: 1/0.01
                                        * Referenced by: '<Root>/Ts'
                                        */
  real_T Kd_d_Gain;                    /* Expression: 0.0005
                                        * Referenced by: '<Root>/Kd_d'
                                        */
  real_T Gain2_Gain;                   /* Expression: 1/100
                                        * Referenced by: '<Root>/Gain2'
                                        */
  real_T Kp_v_Gain;                    /* Expression: 0.000008
                                        * Referenced by: '<Root>/Kp_v'
                                        */
  real_T UnitDelay_InitialCondition;   /* Expression: 0
                                        * Referenced by: '<Root>/Unit Delay'
                                        */
  real_T Gain1_Gain;                   /* Expression: 0.7
                                        * Referenced by: '<Root>/Gain1'
                                        */
  real_T Gain_Gain;                    /* Expression: 0.3
                                        * Referenced by: '<Root>/Gain'
                                        */
  real_T UnitDelay1_InitialCondition;  /* Expression: 0
                                        * Referenced by: '<Root>/Unit Delay1'
                                        */
  real_T Gain5_Gain;                   /* Expression: 1
                                        * Referenced by: '<Root>/Gain5'
                                        */
  real_T Tg_Gain;                      /* Expression: 30
                                        * Referenced by: '<Root>/Tg'
                                        */
  real_T RateLimiter_RisingLim;        /* Expression: 5
                                        * Referenced by: '<Root>/Rate Limiter'
                                        */
  real_T RateLimiter_FallingLim;       /* Expression: -8
                                        * Referenced by: '<Root>/Rate Limiter'
                                        */
  real_T RateLimiter_IC;               /* Expression: 0
                                        * Referenced by: '<Root>/Rate Limiter'
                                        */
  real_T Saturation_UpperSat;          /* Expression: 1
                                        * Referenced by: '<Root>/Saturation'
                                        */
  real_T Saturation_LowerSat;          /* Expression: 0
                                        * Referenced by: '<Root>/Saturation'
                                        */
  real_T Gain4_Gain;                   /* Expression: 100
                                        * Referenced by: '<Root>/Gain4'
                                        */
};

/* Real-time Model Data Structure */
struct tag_RTM_ACC_T {
  const char_T *errorStatus;
  RTWLogInfo *rtwLogInfo;

  /*
   * Timing:
   * The following substructure contains information regarding
   * the timing information for the model.
   */
  struct {
    time_T taskTime0;
    uint32_T clockTick0;
    uint32_T clockTickH0;
    time_T stepSize0;
    time_T tFinal;
    boolean_T stopRequestedFlag;
  } Timing;
};

/* Block parameters (default storage) */
extern P_ACC_T ACC_P;

/* Block states (default storage) */
extern DW_ACC_T ACC_DW;

/* External inputs (root inport signals with default storage) */
extern ExtU_ACC_T ACC_U;

/* External outputs (root outports fed by signals with default storage) */
extern ExtY_ACC_T ACC_Y;

/* Model entry point functions */
extern void ACC_initialize(void);
extern void ACC_step(void);
extern void ACC_terminate(void);

/* Real-time Model object */
extern RT_MODEL_ACC_T *const ACC_M;

/*-
 * The generated code includes comments that allow you to trace directly
 * back to the appropriate location in the model.  The basic format
 * is <system>/block_name, where system is the system number (uniquely
 * assigned by Simulink) and block_name is the name of the block.
 *
 * Use the MATLAB hilite_system command to trace the generated code back
 * to the model.  For example,
 *
 * hilite_system('<S3>')    - opens system 3
 * hilite_system('<S3>/Kp') - opens and selects block Kp which resides in S3
 *
 * Here is the system hierarchy for this model
 *
 * '<Root>' : 'ACC'
 */
#endif                                 /* ACC_h_ */
