/*
 * genx_ids.c
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

#include "genx_ids.h"

#ifdef CAN_IDS_CONFIGURED

#include <string.h>
#include <stdio.h>
#include "ancit_driver_can.h"
#include "genx_can_rx.h"
#include "ancit_can.h"
#include "ancit_common.h"
#include "ancit_timer.h"
/* Added: use the cantools-generated pack function for the IDS_Alert frame
 * instead of hand-rolling the byte layout (see genx_ids_report_violation). */
#include "genx_ids_alert.h"


/***********************************************
 * Timing tracking - last received timestamp per message
 ***********************************************/
static uint32_t ids_last_rx_time[IDS_MSG_VALIDATION_COUNT];

/* Added: the reactive timing check in genx_ids_check_message() only runs when a
 * NEW frame for that ID arrives, so a message that stops arriving entirely
 * (bus-off, ECU killed, suppressed by an attacker) was never flagged - there's
 * no new arrival to trigger the check. These track, per configured message,
 * whether a "gone silent" violation has already been reported for the current
 * outage, so the round-robin poll (genx_ids_poll_next) can report it once
 * instead of spamming every poll cycle while it stays missing. */
static bool ids_missing_reported[IDS_MSG_VALIDATION_COUNT];
static uint8_t ids_poll_idx = 0U;


/***********************************************
 * IDS Whitelist Table
 ***********************************************/
const ids_whitelist_entry_t ids_whitelist[IDS_VALID_ID_COUNT] = {
    { .can_id = 0x201 },
    { .can_id = 0x202 },

};

/***********************************************
 * Per-Signal Range Check Tables
 ***********************************************/
static const ids_signal_range_t ids_signal_ranges_0[] = {
    { .signal_name = "vacc_enable", .min_value = 0.0, .max_value = 3.0 },
    { .signal_name = "acc_enable", .min_value = 0.0, .max_value = 3.0 },
    { .signal_name = "speed_cmd", .min_value = 0.0, .max_value = 100.0 },
    { .signal_name = "direction_cmd", .min_value = 0.0, .max_value = 3.0 },
};

static const ids_signal_range_t ids_signal_ranges_1[] = {
    { .signal_name = "acc_virt_obj_dist", .min_value = 0.0, .max_value = 65535.0 },
    { .signal_name = "acc_d_min", .min_value = 0.0, .max_value = 255.0 },
    { .signal_name = "acc_v_set", .min_value = 0.0, .max_value = 100.0 },
};



/***********************************************
 * Per-Message Validation Configuration
 ***********************************************/
const ids_message_config_t ids_msg_configs[IDS_MSG_VALIDATION_COUNT] = {
    {
        .msg_id = 0x201,
        .dlc_check_enabled = 1,
        .expected_dlc = 2,
        .timing_check_enabled = 1,
        .min_interval_ms = 0,
        .max_interval_ms = 50,
        .signal_range_check_enabled = 1,
        /* Fixed: was 2, which silently skipped speed_cmd/direction_cmd
         * range checks even though both are defined in ids_signal_ranges_0. */
        .signal_count = 4,
        .signal_ranges = ids_signal_ranges_0,
    },
    {
        .msg_id = 0x202,
        .dlc_check_enabled = 1,
        .expected_dlc = 4,
        .timing_check_enabled = 1,
        .min_interval_ms = 0,
        .max_interval_ms = 50,
        .signal_range_check_enabled = 1,
        .signal_count = 3,
        .signal_ranges = ids_signal_ranges_1,
    },

};

/***********************************************
 * IDS Core Functions
 ***********************************************/
void genx_ids_init(void) {
    /* Software Filter Mode: Disable hardware mailbox filtering */
    /* Set mask type to individual so we can control each MB mask */
    for (uint8_t inst = 0; inst < CAN_MAX_INSTANCES; inst++) {
        if (can_active_instances[inst]) {
            ancit_driver_can_SetRxMaskType(inst, FLEXCAN_RX_MASK_INDIVIDUAL);
        }
    }
    /* Set each RX mailbox mask to 0x000 (accept all CAN IDs) */
    for (uint8_t idx = 0; idx < gVars.can_rx_max; idx++) {
        ancit_driver_can_SetRxIndividualMask(can_rx[idx].instance_id,
            can_rx[idx].msg.msg_id_type, can_rx[idx].mb_idx, 0x000);
    }

    memset(ids_last_rx_time, 0, sizeof(ids_last_rx_time));
    /* Added: init state for the round-robin "message gone silent" poll. */
    memset(ids_missing_reported, 0, sizeof(ids_missing_reported));
    ids_poll_idx = 0U;

}

bool genx_ids_check_whitelist(uint32_t msg_id) {
    for (int i = 0; i < IDS_VALID_ID_COUNT; i++) {
        if (ids_whitelist[i].can_id == msg_id) {
            return true;
        }
    }
    return false;
}

void genx_ids_check_message(uint32_t msg_id, uint8_t dlc, uint8_t rx_msg_idx) {
    /* Check whitelist first */
    if (!genx_ids_check_whitelist(msg_id)) {
        genx_ids_report_violation(IDS_VIOLATION_UNKNOWN_ID, msg_id, "Unknown CAN ID");
        return;
    }

    /* Find message config and run per-message checks */
    for (int i = 0; i < IDS_MSG_VALIDATION_COUNT; i++) {
        if (ids_msg_configs[i].msg_id == msg_id) {
            /* Added: the message just arrived, so it's no longer "missing" -
             * clear it unconditionally, regardless of whether this particular
             * arrival was itself early/late. That's a separate concern handled
             * by the reactive timing check right below. */
            ids_missing_reported[i] = false;

            /* DLC Check */
            if (ids_msg_configs[i].dlc_check_enabled && dlc != ids_msg_configs[i].expected_dlc) {
                genx_ids_report_violation(IDS_VIOLATION_DLC_MISMATCH, msg_id, "DLC mismatch");
            }

            /* Timing Check */
                        if (ids_msg_configs[i].timing_check_enabled &&
                            ids_msg_configs[i].max_interval_ms > 0U) {

                            uint32_t now = ancit_GetTick();
                            uint32_t elapsed = now - ids_last_rx_time[i];

                            if (ids_last_rx_time[i] != 0U &&
                                (elapsed < ids_msg_configs[i].min_interval_ms ||
                                 elapsed > ids_msg_configs[i].max_interval_ms)) {
                                genx_ids_report_violation(IDS_VIOLATION_TIMING, msg_id, "Timing violation");
                            } else {
                                /* Reverted: continuous 0x7FF IDS_Alert traffic on every
                                 * in-time message is intentional (used as a heartbeat) -
                                 * kept per user confirmation, not treated as a bug. */
                                genx_ids_report_violation(IDS_VIOLATION_NONE, msg_id, "No violation");
                            }

                            ids_last_rx_time[i] = now;
                        }


                        break;
                    }
                }
            }

/* Added: this was configured (ids_signal_ranges_0/1, signal_range_check_enabled,
 * signal_count) but no function ever actually performed the range check, so
 * IDS_VIOLATION_SIGNAL_RANGE could never be raised. genx_ids_check_message() can't
 * do this itself since it only sees raw msg_id/dlc, not decoded signal values, so
 * the caller must invoke this after unpacking, passing the decoded values in the
 * same order as the msg_id's ids_signal_range_t table. */
void genx_ids_check_signal_ranges(uint32_t msg_id, const double *values, uint8_t value_count) {
    for (int i = 0; i < IDS_MSG_VALIDATION_COUNT; i++) {
        if (ids_msg_configs[i].msg_id == msg_id) {
            if (!ids_msg_configs[i].signal_range_check_enabled) {
                return;
            }

            uint8_t check_count = ids_msg_configs[i].signal_count;
            if (value_count < check_count) {
                check_count = value_count;
            }

            for (uint8_t s = 0; s < check_count; s++) {
                const ids_signal_range_t *range = &ids_msg_configs[i].signal_ranges[s];
                if (values[s] < range->min_value || values[s] > range->max_value) {
                    genx_ids_report_violation(IDS_VIOLATION_SIGNAL_RANGE, msg_id, range->signal_name);
                }
            }
            return;
        }
    }
}

/***********************************************
 * IDS Violation Reporting
 ***********************************************/

/* Added: number of real violations genx_ids_main() can hold for retry while
 * mailbox 15 is busy. Sized for a short burst (e.g. a DLC violation on one
 * message racing a timing violation on another) - not meant for sustained
 * overload, see the queue-full handling in genx_ids_report_violation(). */
#define IDS_PENDING_QUEUE_SIZE 8U

typedef struct {
    ids_violation_type_t type;
    uint32_t msg_id;
} ids_pending_alert_t;

/* Added: ring buffer of violations that lost the race for mailbox 15. */
static ids_pending_alert_t ids_pending_queue[IDS_PENDING_QUEUE_SIZE];
static uint8_t ids_pending_head = 0U;
static uint8_t ids_pending_count = 0U;

/* Added: builds and transmits one IDS_Alert frame, shared by
 * genx_ids_report_violation() (first attempt) and genx_ids_main() (retries),
 * so both paths always produce an identical, correctly-packed frame. */
static status_t genx_ids_send_alert_frame(ids_violation_type_t type, uint32_t msg_id) {
    /* Fixed: previously the alert bytes were packed by hand as MSB-first
     * (byte1=(id>>24)..byte4=id&0xFF), but the cantools-generated unpacker for
     * this same frame (genx_ids_alert_ids_alert_unpack) reconstructs offending_id
     * LSB-first (byte1=id&0xFF..byte4=(id>>24)). Any receiver decoding the frame
     * with the generated code got a byte-swapped CAN ID. Using the generated pack
     * function directly guarantees the wire format always matches the unpacker. */
    struct genx_ids_alert_ids_alert_t alert_msg;
    genx_ids_alert_ids_alert_init(&alert_msg);
    alert_msg.violation_type = genx_ids_alert_ids_alert_violation_type_encode((double)type);
    alert_msg.offending_id = genx_ids_alert_ids_alert_offending_id_encode((double)msg_id);

    uint8_t ids_alert_data[8] = {0};
    genx_ids_alert_ids_alert_pack(ids_alert_data, &alert_msg, sizeof(ids_alert_data));

    flexcan_data_info_t ids_tx_info = { .data_length = 8U, .msg_id_type = FLEXCAN_MSG_ID_STD };
    return ancit_driver_can_send(0, IDS_ALERT_TX_MB, &ids_tx_info, 0x7FF, ids_alert_data);
}

void genx_ids_report_violation(ids_violation_type_t type, uint32_t msg_id, const char *detail) {
    /* Added: no UART/log sink exists in this build for violation details, so
     * `detail` (e.g. "Unknown CAN ID", offending signal name) isn't transmitted
     * anywhere yet. Suppress the unused-parameter warning until logging exists. */
    (void)detail;

    status_t send_status = genx_ids_send_alert_frame(type, msg_id);

    /* Added: IDS_Alert shares one dedicated mailbox (IDS_ALERT_TX_MB) between the
     * "No Violation" heartbeat and every real violation. FLEXCAN_DRV_Send()
     * returns STATUS_BUSY (never queues/overwrites) if a previous alert frame is
     * still transmitting, so without this check a real violation could silently
     * lose the race to a heartbeat, or two violations to each other. */
    if (send_status != STATUS_SUCCESS) {
        if (type == IDS_VIOLATION_NONE) {
            /* Heartbeat only - fine to skip, another one follows on the next
             * in-time message. Not worth displacing a real violation for. */
            return;
        }

        if (ids_pending_count < IDS_PENDING_QUEUE_SIZE) {
            uint8_t tail = (uint8_t)((ids_pending_head + ids_pending_count) % IDS_PENDING_QUEUE_SIZE);
            ids_pending_queue[tail].type = type;
            ids_pending_queue[tail].msg_id = msg_id;
            ids_pending_count++;
        }
        /* else: queue itself is full (sustained overload) - drop the newest
         * violation rather than evict an older one still waiting to be sent. */
    }
}

static void genx_ids_poll_next(void);

/* Added: drains ids_pending_queue (one retry per call) and advances the
 * round-robin "is this configured message still alive" poll (one entry per
 * call). Call once per main loop iteration (see main.c). */
void genx_ids_main(void) {
    if (ids_pending_count != 0U) {
        status_t send_status = genx_ids_send_alert_frame(ids_pending_queue[ids_pending_head].type,
                                                           ids_pending_queue[ids_pending_head].msg_id);
        if (send_status == STATUS_SUCCESS) {
            ids_pending_head = (uint8_t)((ids_pending_head + 1U) % IDS_PENDING_QUEUE_SIZE);
            ids_pending_count--;
        }
        /* else: mailbox still busy - leave it queued and try again next call. */
    }

    genx_ids_poll_next();
}

/* Added: checks ONE configured message per call (round-robin via ids_poll_idx),
 * independent of whether that ID's next frame ever shows up. This is what
 * catches a message that has gone completely silent - the reactive check in
 * genx_ids_check_message() only fires on a new arrival, so a dropped/killed
 * message would otherwise never be flagged at all. Cycling one entry per call
 * (rather than looping the whole table every call) also means a fast-arriving
 * ID (e.g. a 1ms-period message) can't starve a slower/now-silent ID out of
 * being checked - every configured message gets an equal, steady turn. */
static void genx_ids_poll_next(void) {
    if (IDS_MSG_VALIDATION_COUNT == 0) {
        return;
    }

    const ids_message_config_t *cfg = &ids_msg_configs[ids_poll_idx];

    if (cfg->timing_check_enabled && cfg->max_interval_ms > 0U &&
        ids_last_rx_time[ids_poll_idx] != 0U) {

        uint32_t now = ancit_GetTick();
        uint32_t elapsed = now - ids_last_rx_time[ids_poll_idx];

        if (elapsed > cfg->max_interval_ms && !ids_missing_reported[ids_poll_idx]) {
            genx_ids_report_violation(IDS_VIOLATION_TIMING, cfg->msg_id, "Message missing");
            /* Latch so we report the outage once, not on every poll cycle
             * while it stays silent - cleared as soon as it's received again
             * (see genx_ids_check_message). */
            ids_missing_reported[ids_poll_idx] = true;
        }
    }

    ids_poll_idx = (uint8_t)((ids_poll_idx + 1U) % IDS_MSG_VALIDATION_COUNT);
}

/***********************************************
 * IDS Range Filter Initialization (Hardware Mode)
 ***********************************************/


#endif /* CAN_IDS_CONFIGURED */
