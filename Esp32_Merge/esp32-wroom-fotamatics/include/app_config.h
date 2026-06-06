#ifndef APP_CONFIG_H
#define APP_CONFIG_H

// ============================================================
//  FOTAmatics compile-time feature mode
//
//  Pick ONE by setting FOTAMATICS_MODE below:
//
//    APP_MODE_FOTA_ONLY        — FOTA pipeline only.
//                                TelematicsTask is not created.
//                                SmartKit/Control is not subscribed.
//
//    APP_MODE_TELEMATICS_ONLY  — Telematics CAN→MQTT only.
//                                fota_mqtt_init() is not called.
//                                SmartWheelsNS / sdv/vehicles topics
//                                are not subscribed or routed.
//                                FOTA registration check is skipped
//                                so MQTT starts without device.json.
//
//    APP_MODE_BOTH             — Full merged system (default).
//                                FOTA and Telematics coexist on a
//                                single MQTT connection and Serial1.
// ============================================================

#define APP_MODE_FOTA_ONLY        1
#define APP_MODE_TELEMATICS_ONLY  2
#define APP_MODE_BOTH             3

// ---- Change this line to switch modes ----
#define FOTAMATICS_MODE  APP_MODE_BOTH
// ------------------------------------------

// Compile-time guard: catch invalid values
#if FOTAMATICS_MODE != APP_MODE_FOTA_ONLY && \
    FOTAMATICS_MODE != APP_MODE_TELEMATICS_ONLY && \
    FOTAMATICS_MODE != APP_MODE_BOTH
#error "FOTAMATICS_MODE must be APP_MODE_FOTA_ONLY, APP_MODE_TELEMATICS_ONLY, or APP_MODE_BOTH"
#endif

#endif // APP_CONFIG_H
