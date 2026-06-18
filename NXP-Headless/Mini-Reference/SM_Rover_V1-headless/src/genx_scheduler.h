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
 
 * Created on: 15-06-2026
 *     Author: SasiPrasanthSakhinal
 *  
 */

#ifndef GENX_SCHEDULER_H_
#define GENX_SCHEDULER_H_
#include "genx_config.h"
#include "genx_common.h"

#ifdef SCHEDULER_CONFIGURED
#include "ancit_scheduler.h"

#define TASK_Task_OnStart_IDX 		0
#define TASK_Task_1ms_IDX 		1
#define TASK_Task_10ms_IDX 		2


#define MAX_TASKS		3
void Task_While(void);

void genx_scheduler_init(void);
// This macro replaces calls to genx_scheduler_main() with a direct call to ancit_scheduler_main().
#define genx_scheduler_main() ancit_scheduler_main()

#endif //SCHEDULER_CONFIGURED
#endif /* GENX_SCHEDULER_H_ */
