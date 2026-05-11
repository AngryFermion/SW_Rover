# SmartWheels GenX – Project System Requirements

| Field | Value |
|---|---|
| **Project** | SM_Rover_V1 |
| **Target Board** | SmartWheels_Micro_EV2 |
| **ECU Version** | - |
| **Author** | SasiPrasanthSakhinal |
| **Generated** | 2026-05-08 16:22:07 |

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
| Distance | uint8_t | 1 | 0 | NONE |
| V_set | uint8_t | 1 | 0 | NONE |
| D_min | uint8_t | 1 | 0 | NONE |
| PWM | uint8_t | 1 | 0 | NONE |

## 8. UART Configuration

| RTE Variable | Data Type | Size |
|---|---|---|
| Distance | uint8_t | 1 |
| PWM | uint8_t | 1 |

## 9. OLED Display Configuration

12 display item(s) configured.

