/*
 * genx_uart_rte.c
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
#include "genx_uart_rte.h"
#include "ancit_uart_rte.h"
#include "genx_common.h"
#include <string.h>
#include <stdio.h>

void ancit_uart_message_setup(void) {
	char uartBuffer[256];
	uint16_t len = 0;

		len += sprintf(&uartBuffer[len], "Distance,%d\n", ggenx.Ultra_Distance);
	len += sprintf(&uartBuffer[len], "Vset,%d\n", ggenx.Vset);
	len += sprintf(&uartBuffer[len], "Speed,%d\n", ggenx.PWM);
	len += sprintf(&uartBuffer[len], "Dmin,%d\n", ggenx.Dmin);


	ancit_uart_conn_EnqueueString(uartBuffer, len);
}

void ancit_uart_command_receive(char *cmd) {
		// No UART commands configured

}
