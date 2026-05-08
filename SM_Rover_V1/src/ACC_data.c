/*
 * ACC_data.c
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

/* Block parameters (default storage) */
P_ACC_T ACC_P = {
  /* Expression: 0.000005
   * Referenced by: '<Root>/Kp_d'
   */
  0.00005,

  /* Expression: 1/0.01
   * Referenced by: '<Root>/Ts'
   */
  100.0,

  /* Expression: 0.0005
   * Referenced by: '<Root>/Kd_d'
   */
  0.0005,

  /* Expression: 1/100
   * Referenced by: '<Root>/Gain2'
   */
  0.01,

  /* Expression: 0.000008
   * Referenced by: '<Root>/Kp_v'
   */
  0.0005,

  /* Expression: 0
   * Referenced by: '<Root>/Unit Delay'
   */
  0.0,

  /* Expression: 0.7
   * Referenced by: '<Root>/Gain1'
   */
  0.7,

  /* Expression: 0.3
   * Referenced by: '<Root>/Gain'
   */
  0.3,

  /* Expression: 0
   * Referenced by: '<Root>/Unit Delay1'
   */
  0.0,

  /* Expression: 1
   * Referenced by: '<Root>/Gain5'
   */
  1.0,

  /* Expression: 12
   * Referenced by: '<Root>/Tg'
   */
  20.0,

  /* Expression: 5
   * Referenced by: '<Root>/Rate Limiter'
   */
  5.0,

  /* Expression: -8
   * Referenced by: '<Root>/Rate Limiter'
   */
  -8.0,

  /* Expression: 0
   * Referenced by: '<Root>/Rate Limiter'
   */
  0.0,

  /* Expression: 1
   * Referenced by: '<Root>/Saturation'
   */
  1.0,

  /* Expression: 0
   * Referenced by: '<Root>/Saturation'
   */
  0.0,

  /* Expression: 100
   * Referenced by: '<Root>/Gain4'
   */
  100.0
};
