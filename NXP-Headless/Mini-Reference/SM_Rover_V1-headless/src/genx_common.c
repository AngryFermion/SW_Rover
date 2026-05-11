/*
 * genx_common.c
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
 
 * Created on: 08-05-2026
 *     Author: SasiPrasanthSakhinal
 *  
 */
#include "genx_config.h"

#include "genx_common.h"
#include "string.h"
 
global_genx_t ggenx;
 
void genx_global_init(void) {


	ggenx.Distance		= 		0;
	ggenx.V_set			= 		100;
	ggenx.D_min			= 		50;
	ggenx.PWM			= 		0;


}

void genx_clock_init(void) {
	/* Write your local variable definition here */
	status_t status;

	/* Initialize clock module */
	status = CLOCK_DRV_Init(&clockMan1_InitConfig0);
	DEV_ASSERT(status == STATUS_SUCCESS);

	/* Initialize pins
	 *  -   Init FlexCAN and GPIO pins
	 *  -   See PinSettings component for more info
	 */
	PINS_DRV_Init(NUM_OF_CONFIGURED_PINS0, g_pin_mux_InitConfigArr0);

	/* Initialize and configure clocks
	 *  -   Setup system clocks, dividers
	 *  -   Configure FlexCAN clock, GPIO
	 *  -   see clock manager component for more details
	 */
	CLOCK_DRV_Init(&clockMan1_InitConfig0);

	CLOCK_SYS_Init(g_clockManConfigsArr, CLOCK_MANAGER_CONFIG_CNT,
			g_clockManCallbacksArr, CLOCK_MANAGER_CALLBACK_CNT);
	CLOCK_SYS_UpdateConfiguration(0U, CLOCK_MANAGER_POLICY_AGREEMENT);
}
