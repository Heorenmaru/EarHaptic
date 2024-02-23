/*
 * au_logic.h
 *
 *  Created on: Mar 16, 2022
 *      Author: Heorenmaru
 */

#ifndef __KERNEL_H___
#define __KERNEL_H__


//#include "protocol.h"

//#include "radioconf.h"


#include "usbd_cdc_if.h"
#include "usb_device.h"

#include "handlers.h"


///HARDWARE CHECK STATUS CODE
#define AU_HW_OK					0
#define AU_HW_LORA					1
#define AU_HW_GPS					2
#define AU_HW_BAROMETER				3
#define AU_HW_ACCEL					4
#define AU_HW_BAT_VOLTAGE			5

void kernel_init();
void kernel_main();




#define PWR_RF				17U







#endif /* INC_KERNEL_H_ */
