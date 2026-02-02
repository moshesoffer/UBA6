/*
 * UBA_battary.h
 *
 *  Created on: Oct 13, 2024
 *      Author: ORA
 */

#ifndef UBA_BATTARY_H_
#define UBA_BATTARY_H_

#define UBA_BAT_SN_MAX_LENGTH (10)
typedef enum UBA_BATTERY_TYPE {
	UBA_BATTERY_TYPE_NONE,
	UBA_BATTERY_TYPE_PRIMERY,
	UBA_BATTERY_TYPE_SECONDERY,
	UBA_BATTERY_TYPE_MAX,
	UBA_BATTERY_TYPE_INVALID
} UBA_BATTERY_TYPE;

typedef enum UBA_CELL_CHEMISTRY {
	UBA_CELL_CHEMISTRY_LITHIUM_ION,
	UBA_CELL_CHEMISTRY_CUSTOM = 0xff
} UBA_CELL_CHEMISTRY;

typedef struct UBA_cell {
	UBA_BATTERY_TYPE type;
	uint8_t chemistry;
	uint8_t manufacturer[32];
	uint8_t part_number[32];
	struct {
		int32_t min;
		int32_t nominal;
		int32_t max;
	} voltage;//in mV
	struct {
		int32_t min;
		int32_t max;
	} capacity;//in mAh
	struct {
		int32_t min;
		int32_t max;
	} temperature;//in c
} UBA_cell;

#define UBA_BAT_DEMO			\
{								\
	Battery_TYPE_TYPE_SECONDERY, \
	"123456789",				\
	2,							\
	6300						\
}

#endif /* UBA_BATTARY_H_ */
