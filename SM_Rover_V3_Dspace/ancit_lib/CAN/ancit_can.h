#ifndef ANCIT_CAN_GENERATED_H
#define ANCIT_CAN_GENERATED_H
#include "genx_config.h"

#define CAN_MAX_INSTANCES 3 // Maximum number of CAN instances supported

extern bool can_active_instances[CAN_MAX_INSTANCES];

void ancit_can_main(void);
void ancit_can_start(void);
void ancit_can_get_all_instances(void);
#endif
