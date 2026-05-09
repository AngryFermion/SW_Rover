/*
 * genx_do.h
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
 
 * Created on: 10-06-2025
 *     Author: Narayan
 *  
 */

#ifndef GENX_DO_H_
#define GENX_DO_H_
#include "genx_config.h"

#ifdef DIGITAL_OUTPUT_CONFIGURED
#include "ancit_do.h"
/***********************************************
 * ANCIT_CG_Define_Start
 ***********************************************/
#define DO_LED_RED_IDX 		0
#define DO_LED_GREEN_IDX 	1

#define DIGITAL_OUTPUT_MAX		2

/***********************************************
 * ANCIT_CG_Define_End
 ***********************************************/
/***********************************************
 * ANCIT_CG_Digital_Output_Getters_And_Setters_Start
 ***********************************************/
void DO_LED_RED_set_value(uint8_t value);
void DO_LED_RED_toggle(void);

void DO_LED_GREEN_set_value(uint8_t value);
void DO_LED_GREEN_toggle(void);


/***********************************************
 * ANCIT_CG_Digital_Output_Getters_And_Setters_End
 ***********************************************/

void genx_do_init(void);
// This macro replaces calls to genx_do_main() with a direct call to ancit_digital_output_main().
#define genx_do_main() ancit_digital_output_main()

#endif //DIGITAL_OUTPUT_CONFIGURED
#endif /* GENX_DO_H_ */
