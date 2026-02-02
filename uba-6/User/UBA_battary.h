/*
 * UBA_battary.h
 *
 *  Created on: Oct 13, 2024
 *      Author: ORA
 */

#ifndef UBA_BATTARY_H_
#define UBA_BATTARY_H_

#define UBA_BAT_SN_MAX_LENGTH (16)
typedef enum UBA_BATTERY_TYPE{
	UBA_BATTERY_TYPE_NONE,
	UBA_BATTERY_TYPE_PRIMERY,
	UBA_BATTERY_TYPE_SECONDERY,
	UBA_BATTERY_TYPE_MAX,
	UBA_BATTERY_TYPE_INVALID
}UBA_BATTERY_TYPE;

typedef struct UBA_battery{
	UBA_BATTERY_TYPE type;
	char serial_number[UBA_BAT_SN_MAX_LENGTH];
	uint8_t number_of_cells;
	uint32_t max_voltage; // in mV
}UBA_battery;

#define UBA_BAT_DEMO			\
{								\
	UBA_BATTERY_TYPE_SECONDERY, \
	"123456789P",				\
	2,							\
	6.3							\
}

#endif /* UBA_BATTARY_H_ */
