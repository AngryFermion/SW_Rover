# SmartWheels GenX – Project System Requirements

| Field | Value |
|---|---|
| **Project** | SM_Rover_V2 |
| **Target Board** | SmartWheels_Micro_EV2 |
| **ECU Version** | - |
| **Author** | SasiPrasanthSakhinal |
| **Generated** | 2026-08-19 14:46:07 |

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
| H-Bridge Configuration | ? | ? |

## 2. Hardware – IO Peripheral Configuration

| Name | Type | Pin | Port | Channel |
|---|---|---|---|---|
| LED_RED | DIGITAL_OUTPUT | 15 | PTD | - |
| LED_GREEN | DIGITAL_OUTPUT | 16 | PTD | - |
| LED_BLUE | DIGITAL_OUTPUT | 0 | PTD | - |
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
| VACC | uint8_t | 1 | 0 | NONE |
| ACC | uint8_t | 1 | 0 | NONE |
| Speed | uint8_t | 1 | 0 | NONE |
| Mode | uint8_t | 1 | 0 | NONE |
| ACC_Obj | uint8_t | 1 | 0 | NONE |
| ACC_Vset | uint8_t | 1 | 0 | NONE |
| ACC_Dmin | uint8_t | 1 | 0 | NONE |

## 6. CAN Communication

### 6.1 CAN Tx Messages

| Message Name | CAN ID | DLC | Cycle (ms) | Signals |
|---|---|---|---|---|
| ROVER_ACC_STATUS | 0x0x301 | 5 | 10 | RealDmin, RealVset, RealObj, PWM |

### 6.2 CAN Rx Messages

| Message Name | CAN ID | DLC | Cycle (ms) | Signals |
|---|---|---|---|---|
| ROVER_VEHICLE_CMD | 0x0x201 | 2 | 0 | VACC_Enable, ACC_Enable, SpeedCmd, DirectionCmd |
| ROVER_ACC_PARAM | 0x0x202 | 4 | 0 | ACC_VirtObjDist, ACC_DMin, ACC_VSet |

## 8. UART Configuration

| RTE Variable | Data Type | Size |
|---|---|---|
| Ultra_Distance | uint16_t | 1 |
| Vset | uint8_t | 1 |
| PWM | uint8_t | 1 |
| Dmin | uint8_t | 1 |

## 9. OLED Display Configuration

12 display item(s) configured.

