/*
 * genx_runnable.c
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
 
 * Created on: 01-07-2025
 *     Author: Narayan
 *  
 */
#include "genx_runnables.h"

#include "genx_do.h" 


#ifdef RUNNABLES_CONFIGURED

/***********************************************
 * ANCIT_CG_Runnable_Function_Start
 ***********************************************/
 
void Runnable_r1(void) {
//Variable Definition
uint16_t var_outputLED_RED= 0;


	//Receive Input from >>  
//Send Output to >> LED_RED 
var_outputLED_RED = var_input;
// DO_LED_RED_set_value(var_outputLED_RED);

} 



/***********************************************
 * ANCIT_CG_Runnable_Function_End
 ***********************************************/

#endif
