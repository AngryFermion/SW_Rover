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

 * Created on: 08-05-2026
 *     Author: SasiPrasanthSakhinal
 *
 */
#include "genx_uart_rte.h"
#include "genx_common.h"
#include <string.h>
#include <stdio.h>

void ancit_uart_message_setup(void) {
	/* Queue RTE variables to UART as CSV */
	char uartBuffer[256];
	uint8_t len = 0;

	/* Start character */
	//uartBuffer[len++] = '!';
	//uartBuffer[len++] = ',';

		len += sprintf(&uartBuffer[len], "obstruction, %d,", ggenx.Distance);
	    len += sprintf(&uartBuffer[len], "PWM, %d", ggenx.PWM);


	//uartBuffer[len++] = '\r';
	uartBuffer[len++] = '\n';

	ancit_uart_conn_EnqueueString(uartBuffer, len);
}
