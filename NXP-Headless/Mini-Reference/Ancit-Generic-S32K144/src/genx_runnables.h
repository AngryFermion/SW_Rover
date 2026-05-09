/*
 * genx_scheduler.h
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

#ifndef GENX_RUNNABLES_H_
#define GENX_RUNNABLES_H_
#include "genx_config.h"

#ifdef RUNNABLES_CONFIGURED



void Runnable_r1(void);
void Runnable_r2(void);
void Runnable_r3(void);
void Runnable_100ms(void);
void Runnable_10ms(void);
void Runnable_1000ms(void);
void Runnable_500ms(void);
#endif

#endif /* GENX_RUNNABLES_H_ */
