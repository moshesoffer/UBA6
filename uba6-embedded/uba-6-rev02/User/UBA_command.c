/*
 * UBA_command.c
 *
 *  Created on: Nov 20, 2024
 *      Author: ORA
 */

#include "UBA_command.h"
#include "UBA_util.h"
#include "UBA_channel.h"
#include "UBA_DC_DC_converter.h"
#include "UBA_6.h"
#include "UBA_PROTO_CMD.pb.h"
#include "UBA_battery_performance_test.h"
#include "UBA_file_manager.h"

#define UBA_COMP "CMD"

UBA_BPT* UBA_CMD_select_bpt(UBA_CHANNLE_ID ch_id) {
	switch (ch_id) {
		case UBA_CHANNLE_ID_A:
			UBA_6_set_next_state(&UBA_6_device_g, UBA_6_STATE_SINGLE_CHANNELS);
			return &UBA_6_device_g.BPT_A;
			break;
		case UBA_CHANNLE_ID_B:
			UBA_6_set_next_state(&UBA_6_device_g, UBA_6_STATE_SINGLE_CHANNELS);
			return &UBA_6_device_g.BPT_B;
			break;
		case UBA_CHANNLE_ID_AB:
			UBA_6_set_next_state(&UBA_6_device_g, UBA_6_STATE_DUAL_CHANNEL);
			return &UBA_6_device_g.BPT_AB;
			break;
		default:
			UART_LOG_ERROR(UBA_COMP, "The Selected Channel is not define");
			return NULL;
	}
}

UBA_STATUS_CODE UBA_COMMAND_execute(UBA_PROTO_CMD_command_message *cmd) {
	UBA_channel *ch = NULL;
	UBA_BPT * bpt = NULL;
	switch (cmd->which_command) {
		case UBA_PROTO_CMD_command_message_line_tag:
			UBA_line *line = cmd->command.line.line_id == UBA_PROTO_LINE_ID_A ? &UBA_LINE_A : &UBA_LINE_B;
			UBA_line_command_execute(line, &cmd->command.line);
			break;
		case UBA_PROTO_CMD_command_message_channel_tag:
			switch (cmd->command.channel.channel) {
				case UBA_PROTO_CHANNEL_ID_A:
					ch = &UBA_CH_A;
					break;
				case UBA_PROTO_CHANNEL_ID_B:
					ch = &UBA_CH_B;
					break;
				case UBA_PROTO_CHANNEL_ID_AB:
					ch = &UBA_CH_AB;
					break;
				default:
					break;
			}
			UART_LOG_INFO(UBA_COMP, "Channel Command Message");
			UBA_channel_command_execute(ch, &cmd->command.channel);
			break;
		case UBA_PROTO_CMD_command_message_uba_tag:
			UART_LOG_INFO(UBA_COMP, "UBA Command Message :%u",cmd->command.uba.id);
			UBA_6_command_execute(&UBA_6_device_g, &cmd->command.uba);
			break;
		case UBA_PROTO_CMD_command_message_bpt_tag:
			UART_LOG_INFO(UBA_COMP, "BPT Command Message");
			bpt = UBA_CMD_select_bpt((UBA_CHANNLE_ID) cmd->command.bpt.channel);
			if(bpt != NULL){
				UBA_BPT_command_execute(bpt,&cmd->command.bpt);
			}
			break;
		case UBA_PROTO_CMD_command_message_file_tag:
			UART_LOG_INFO(UBA_COMP, "File Manager Command Message");
			UBA_FM_command_execute(&cmd->command.file);
			break;
		default:
			break;
	}
	return UBA_STATUS_CODE_OK;
}
