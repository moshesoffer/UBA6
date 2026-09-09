/*
 * UBA_command.h
 *
 *  Created on: Nov 20, 2024
 *      Author: ORA
 */

#ifndef UBA_COMMAND_H_
#define UBA_COMMAND_H_
#include "stdint.h"
#include "UBA_common_def.h"
#include "UBA_channel.h"
#include "UBA_PROTO_CMD.pb.h"

UBA_STATUS_CODE UBA_COMMAND_execute(UBA_PROTO_CMD_command_message * cmd);

#endif /* UBA_COMMAND_H_ */
