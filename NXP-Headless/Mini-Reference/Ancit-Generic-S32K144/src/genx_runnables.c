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
 
 * Created on: 10-06-2025
 *     Author: Narayan
 *  
 */
#include "genx_runnables.h"

#include "genx_do.h" 
#include "ancit_head_light.h"
#include "genx_di.h"

extern int count;
extern uint8_t button_status;


#ifdef RUNNABLES_CONFIGURED

/***********************************************
 * ANCIT_CG_Runnable_Function_Start
 ***********************************************/
void Runnable_100ms(void) {
	// DO_LED_RED_toggle();
	// DO_LED_GREEN_toggle();
	DO_LED_RED_toggle();
#ifdef HEAD_LIGHT_CONFIGURED
	//  ancit_hl_ApplyRightLightStatus();
	//  ancit_hl_UpdatePwmFromLeveller();
#endif
}
bool led = 0;
void Runnable_1000ms(void) {

	// increment counter
	// counter reaches 10 - 10 seconds
	//

	// DO_LED_GREEN_set_value(1);
	led = !(led);

	
	
	// DO_LED_RED_set_value(led);

#ifdef AUTO_DIP_FEATURE_ENABLED

	if(button_status == 1){
		ancit_tunnel_started(); // geo-fencing started
		count = count + 1;
		if(count > TUNNEL_DURATION){
			ancit_tunnel_crossed();
			button_status = 0;
		}
	}
	else{
		count = 0;
		ancit_tunnel_crossed();
	}

#endif


}

void Runnable_10ms(void) {
// #ifdef HEAD_LIGHT_CONFIGURED
// 	 ancit_hl_ReadAllSensors();
// #endif
// 	 ancit_button_read();
}

void Runnable_500ms(void) {

// #ifdef HEAD_LIGHT_CONFIGURED
// 	 ancit_hl_ToggleIndicators();
// #endif
}

/***********************************************
 * ANCIT_CG_Runnable_Function_End
 ***********************************************/

#endif
