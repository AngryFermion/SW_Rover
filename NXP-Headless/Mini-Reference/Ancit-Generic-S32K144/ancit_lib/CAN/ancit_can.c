#include <ancit_driver_can.h>
#include "ancit_can.h"
#include "ancit_can_rx.h"
#include "ancit_can_tx.h"

//Starts all the modules, Both Tx and Rx part
void ancit_can_start(void) {
#if defined(CAN_TX_CONFIGURED) || defined(CAN_RX_CONFIGURED)
//	/* Initialize the CAN Hardware */
	ancit_driver_can_init(INST_FLEXCAN_CONFIG_1, &flexcanState0,
			&flexcanInitConfig0);
#endif
}

