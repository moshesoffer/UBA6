/*
 * UBA_common_def.h
 *
 *  Created on: Oct 12, 2024
 *      Author: ORA
 */

#ifndef UBA_COMMON_DEF_H_
#define UBA_COMMON_DEF_H_

// @formatter:off
#define UBA_INFO	 	(0x10000000)
#define UBA_WARNING 	(0x20000000)
#define UBA_ERROR 		(0x40000000)
#define UBA_CRITICAL 	(0x80000000)
// @formatter:on
typedef enum UBA_STATUS_CODE {
	UBA_STATUS_CODE_OK = 0x00, /*there is not error*/
	UBA_STATUS_CODE_PARMETER = 0x01, /*one of the parameter are illegal*/
	UBA_STATUS_CODE_CONFIG = 0x02, /*the configuration is illegal */
	UBA_STATUS_NULL = 0x03, /*the object is null*/
	UBA_STATUS_CODE_LIMIT = 0x04, /*the */
	UBA_STATUS_CODE_BUSY = 0x05, /*the */
} UBA_STATUS_CODE;

#endif /* UBA_COMMON_DEF_H_ */

