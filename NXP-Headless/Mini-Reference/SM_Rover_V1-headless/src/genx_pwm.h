/*
 * genx_pwm.h
 *
 * Copyright (c) 2024-2025 ANCIT Consulting Pvt Ltd
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 
 * Created on: 15-06-2026
 *     Author: SasiPrasanthSakhinal
 *  
 */
 #ifndef _GENX_PWM_H_
#define _GENX_PWM_H_

#include "genx_config.h"
#include "ancit_pwm.h"

#ifdef PWM_CONFIGURED
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define TOTAL_PWM_INSTANCES 1 /* Total number of FTM instances used */

extern PWM_registration_params_t genx_pwm_reg[];

/*******************************************************************************
 * API Functions
 ******************************************************************************/
void genx_PWM_LFM_EnableOutput(void);
void genx_PWM_LFM_DisableOutput(void);
void genx_PWM_LFM_updateDutyCycle(uint8_t dutyPercent);
void genx_PWM_LBM_EnableOutput(void);
void genx_PWM_LBM_DisableOutput(void);
void genx_PWM_LBM_updateDutyCycle(uint8_t dutyPercent);
void genx_PWM_RFM_EnableOutput(void);
void genx_PWM_RFM_DisableOutput(void);
void genx_PWM_RFM_updateDutyCycle(uint8_t dutyPercent);
void genx_PWM_RBM_EnableOutput(void);
void genx_PWM_RBM_DisableOutput(void);
void genx_PWM_RBM_updateDutyCycle(uint8_t dutyPercent);


void genx_PWM_Init(void);

#endif

#endif /* _GENX_PWM_H_ */
