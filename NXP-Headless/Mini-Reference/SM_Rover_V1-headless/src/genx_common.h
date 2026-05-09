/*
 * genx_common.h
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

#ifndef GENX_COMMON_H_
#define GENX_COMMON_H_
#include "stdint.h"

typedef struct {
		uint8_t Distance;
	uint8_t V_set;
	uint8_t D_min;
	uint8_t PWM;

} global_genx_t;
 
extern global_genx_t ggenx;

void genx_global_init(void);
void genx_clock_init(void);

#endif /* GENX_COMMON_H_ */
