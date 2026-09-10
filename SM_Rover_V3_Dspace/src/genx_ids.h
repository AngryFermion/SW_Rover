/*
 * genx_ids.h
 *
 * CAN Intrusion Detection System (IDS)
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

 * Created on: 08-09-2026
 *     Author: User
 *
 */

#ifndef GENX_IDS_H_
#define GENX_IDS_H_

#include "genx_config.h"

#ifdef CAN_IDS_CONFIGURED

#include <stdint.h>
#include <stdbool.h>

/***********************************************
 * IDS Configuration Defines
 ***********************************************/
#define IDS_VALID_ID_COUNT 2

#define IDS_MSG_VALIDATION_COUNT 2

#define IDS_ALERT_TX_MB  15U  /* Dedicated TX mailbox for IDS alert messages */

/***********************************************
 * IDS Data Types
 ***********************************************/
typedef enum {
    IDS_VIOLATION_NONE = 0,
    IDS_VIOLATION_UNKNOWN_ID,
    IDS_VIOLATION_DLC_MISMATCH,
    IDS_VIOLATION_TIMING,
    IDS_VIOLATION_SIGNAL_RANGE
} ids_violation_type_t;

typedef struct {
    const char *signal_name;
    double min_value;
    double max_value;
} ids_signal_range_t;

typedef struct {
    uint32_t msg_id;
    uint8_t dlc_check_enabled;
    uint8_t expected_dlc;
    uint8_t timing_check_enabled;
    uint32_t min_interval_ms;
    uint32_t max_interval_ms;
    uint8_t signal_range_check_enabled;
    uint8_t signal_count;
    const ids_signal_range_t *signal_ranges;
} ids_message_config_t;

typedef struct {
    uint32_t can_id;
} ids_whitelist_entry_t;

/***********************************************
 * IDS Range Filter Definitions (Hardware Mode)
 ***********************************************/


/***********************************************
 * IDS Extern Declarations
 ***********************************************/
extern const ids_whitelist_entry_t ids_whitelist[];
extern const ids_message_config_t ids_msg_configs[];

/***********************************************
 * IDS Function Prototypes
 ***********************************************/
void genx_ids_init(void);
bool genx_ids_check_whitelist(uint32_t msg_id);
void genx_ids_check_message(uint32_t msg_id, uint8_t dlc, uint8_t rx_msg_idx);
void genx_ids_report_violation(ids_violation_type_t type, uint32_t msg_id, const char *detail);
/* Added: real violations that lost the race for the shared IDS_Alert mailbox
 * (STATUS_BUSY) are queued instead of dropped. Call once per main loop
 * iteration, like the other genx_*_main() functions, to retry/drain that queue. */
void genx_ids_main(void);
/* Added: performs the per-signal min/max range check that was configured
 * (ids_signal_ranges_*) but never actually invoked. Call after the message
 * has been unpacked, passing the decoded signal values in the same order
 * as the corresponding ids_signal_range_t table for that msg_id. */
void genx_ids_check_signal_ranges(uint32_t msg_id, const double *values, uint8_t value_count);



#endif /* CAN_IDS_CONFIGURED */
#endif /* GENX_IDS_H_ */
