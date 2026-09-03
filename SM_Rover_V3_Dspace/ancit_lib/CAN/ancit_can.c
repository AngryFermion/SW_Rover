#include <ancit_driver_can.h>
#include "ancit_can.h"
#include "ancit_can_rx.h"
#include "ancit_can_tx.h"
#include "ancit_common.h"

#if defined(CAN_TX_CONFIGURED) || defined(CAN_RX_CONFIGURED)

#ifdef CAN_TX_CONFIGURED
extern can_tx_message_registration_params_t tx_reg[];
#endif

#ifdef CAN_RX_CONFIGURED
extern can_rx_message_registration_params_t rx_reg[];
#endif

bool can_active_instances[CAN_MAX_INSTANCES];
extern global_common_vars_t gVars;

// Function to initialize CAN instances based on instance_id
void ancit_can_init_instances(void) {
	// First, get all active instances from both TX and RX
	ancit_can_get_all_instances();
	
	// Initialize only the active instances
	for (uint8_t instance_id = 0; instance_id < CAN_MAX_INSTANCES; instance_id++) {
		if (can_active_instances[instance_id]) {
			// Initialize the CAN instance based on instance_id
			switch (instance_id) {
#ifdef INST_FLEXCAN_CONFIG_0
				case 0:
					ancit_driver_can_init(INST_FLEXCAN_CONFIG_0, &flexcanState0, &flexcanInitConfig0);
                    break;
#endif
#ifdef INST_FLEXCAN_CONFIG_1
				case 1:
                    ancit_driver_can_init(INST_FLEXCAN_CONFIG_1, &flexcanState1, &flexcanInitConfig1);
					break;
#endif
#ifdef INST_FLEXCAN_CONFIG_2
				case 2:
					ancit_driver_can_init(INST_FLEXCAN_CONFIG_2, &flexcanState2, &flexcanInitConfig2);
					break;
#endif
				default:
					// Handle unsupported instance IDs
					break;
			}
		}
	}
}

//Starts all the modules, Both Tx and Rx part
void ancit_can_start(void) {
    // Initialize CAN instances dynamically based on tx_reg array
    ancit_can_init_instances();

#ifdef CAN_RX_CONFIGURED
	//Initialize the CAN Rx
	ancit_can_rx_init();
#endif //CAN_RX_CONFIGURED

#ifdef CAN_TX_CONFIGURED
	//Initialize the CAN Tx
	ancit_can_tx_init();

	//Check for Onstart messages and transmit them
	ancit_can_tx_onstart();
#endif //CAN_TX_CONFIGURED

}

void ancit_can_get_all_instances(void)
{
	// Reset all instances to inactive
	for (uint8_t i = 0; i < CAN_MAX_INSTANCES; i++)
	{
		can_active_instances[i] = false;
	}
	
#ifdef CAN_TX_CONFIGURED
	// Mark TX instances as active
	for (uint8_t can_idx = 0; can_idx < gVars.can_tx_max; can_idx++)
	{
		const can_tx_message_registration_params_t *reg = &tx_reg[can_idx];
		
		// Mark this instance as active if valid
		if (reg->instance_id < CAN_MAX_INSTANCES)
		{
			can_active_instances[reg->instance_id] = true;
		}
	}
#endif // CAN_TX_CONFIGURED

#ifdef CAN_RX_CONFIGURED
	// Mark RX instances as active
	for (uint8_t can_idx = 0; can_idx < gVars.can_rx_max; can_idx++)
	{
		const can_rx_message_registration_params_t *reg = &rx_reg[can_idx];
		
		// Mark this instance as active if valid
		if (reg->instance_id < CAN_MAX_INSTANCES)
		{
			can_active_instances[reg->instance_id] = true;
		}
	}
#endif // CAN_RX_CONFIGURED
}

#endif
