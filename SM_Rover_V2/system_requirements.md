# SmartWheels GenX – Project System Requirements

| Field | Value |
|---|---|
| **Project** | SM_Rover_V2 |
| **Target Board** | SmartWheels_Micro_EV2 |
| **ECU Version** | - |
| **Author** | SasiPrasanthSakhinal |
| **Generated** | 2026-06-15 16:21:37 |

---

## 1. Active Feature Modules

| Feature Module | Allowed | Enabled |
|---|:---:|:---:|
| IO Configuration | ? | ? |
| Application Data Configuration | ? | ? |
| Application Configuration | ? | ? |
| Scheduler Configuration | ? | ? |
| CAN Tx Configuration | ? | ? |
| CAN Rx Configuration | ? | ? |
| NVM Configuration | ? | ? |
| CAN IDS Configuration | ? | ? |
| OLED Display Configuration | ? | ? |
| UART Configuration | ? | ? |
| Bootloader Settings | ? | ? |
| UDS DID Configuration | ? | ? |
| Telematics | ? | ? |
| CAN Gateway | ? | ? |
| Simulink Integration | ? | ? |
| Resources | ? | ? |

## 2. Hardware – IO Peripheral Configuration

| Name | Type | Pin | Port | Channel |
|---|---|---|---|---|
| LFM | PWM_OUTPUT | 8 | PTB | - |
| LBM | PWM_OUTPUT | 9 | PTB | - |
| RFM | PWM_OUTPUT | 2 | PTD | - |
| RBM | PWM_OUTPUT | 3 | PTD | - |

## 3. Task Scheduler

| Task Name | Frequency (ms) | Assigned Runnables |
|---|---|---|
| Task_OnStart | 0 | - |
| Task_1ms | 1 | - |
| Task_10ms | 10 | - |

## 5. Application Data – RTE Variables

| Variable Name | Data Type | Size | Default Value | Type |
|---|---|---|---|---|
| Ultra_Distance | uint16_t | 1 | 0 | NONE |
| Vset | uint8_t | 1 | 0 | NONE |
| PWM | uint8_t | 1 | 0 | NONE |
| Dmin | uint8_t | 1 | 0 | NONE |

## 8. UART Configuration

| RTE Variable | Data Type | Size |
|---|---|---|
| Ultra_Distance | uint16_t | 1 |
| Vset | uint8_t | 1 |
| PWM | uint8_t | 1 |
| Dmin | uint8_t | 1 |

## 9. OLED Display Configuration

12 display item(s) configured.

