/*
 * UBA_UART_comm.c
 *
 *  Created on: Feb 6, 2025
 *      Author: ORA
 */

#include "UBA_UART_comm.h"
#include <stdio.h>
#include <pb_encode.h>
#include <pb_decode.h>
#include "simple.pb.h"
#include "uart_log.h"
#include "stm32g4xx_hal.h"
#include "string.h"

#include "UBA_6.h"
#include "UBA_test_routine.h"
#include "UBA_Message.pb.h"
#include "UBA_channel.h"
#include "UBA_command.h"
#include "file_logger.h"
#include "UBA_PROTO_helper.h"
#include "UBA_file_manager.h"

#define COMP "UBA_UART_Comm"

#define UART_RX_BUFFER_SIZE 4096
#define UART_RX_VARINT_VALUE_MASK 0x7F
#define UART_RX_VARINT_FLAG_MASK 0x80

uint8_t uart_rx_dma_buffer[UART_RX_BUFFER_SIZE];
uint8_t uart_tx_dma_buffer[UART_RX_BUFFER_SIZE];
volatile uint16_t old_pos = 0;
uint16_t frame_index = 0;
bool in_frame = false;
static bool DMA_done = false;
static UBA_UART_QUERY_RECIPIENT query_pending_reqest = UBA_UART_QUERY_RECIPIENT_NONE;

static uint32_t query_pending_id = 0;

#if (UBA_LOG_LEVEL_COMM <= UART_LOG_LEVEL_INFO)
#define UART_LOG_COMM_INFO(...) UART_LOG_INFO(COMP,##__VA_ARGS__)
#else
#define UART_LOG_COMM_INFO(...)
#endif

#if (UBA_LOG_LEVEL_COMM <= UART_LOG_LEVEL_DEBUG)
#define UART_LOG_COMM_DEBUG(...) UART_LOG_DEBUG(COMP,##__VA_ARGS__)
#else
#define UART_LOG_COMM_DEBUG(...)
#endif
// ===================== private functions declarations =====================
void UBA_UART_query_response_message(UBA_UART_QUERY_RECIPIENT id);
void process_uart_data(uint8_t *data, uint16_t len);

//===================== public functions functions =====================
void UBA_UART_qeury_pending_post(UBA_UART_QUERY_RECIPIENT new_query_request, uint32_t query_id) {
	query_pending_reqest |= new_query_request;
	query_pending_id = query_id;

}
void UBA_UART_qeury_pending_clear(UBA_UART_QUERY_RECIPIENT clear_query_request) {
	query_pending_reqest = (query_pending_reqest & (~clear_query_request));
}

bool uart_write_callback(pb_ostream_t *stream, const uint8_t *buf, size_t count) {
	if (HAL_UART_Transmit(&COMM_CH, (uint8_t*) buf, count, HAL_MAX_DELAY) != HAL_OK) {
		return false;
	}
	return true;
}

pb_ostream_t uart_ostream = { .callback = uart_write_callback, .state = NULL, .max_size = SIZE_MAX, .bytes_written = 0 };

bool uart_read_callback(pb_istream_t *stream, uint8_t *buf, size_t count) {
	if (HAL_UART_Receive(&COMM_CH, buf, count, HAL_MAX_DELAY) != HAL_OK)
		return false;
	return true;
}

void UBA_UART_print_buffer(uint8_t *buffer, uint32_t size) {
//#if (UBA_LOG_LEVEL_COMM <= UART_LOG_LEVEL_DEBUG)
	UART_LOG_COMM_DEBUG("Message:");
	for (size_t i = 0; i < size; i++) {
		uart_printf("%02X ", buffer[i]);
	}
	uart_printf("\n");
//#endif
}

size_t encode_varint(uint32_t value, uint8_t *output) {
	size_t i = 0;
	while (value > 127) {
		output[i++] = (value & 0x7F) | 0x80;
		value >>= 7;
	}
	output[i++] = value;
	return i;
}

bool UBA_UART_sent_message(MSG_Message *message, int tag) {
	size_t message_size = 0, index;
	bool status;
	HAL_UART_StateTypeDef ret = HAL_UART_GetState(&COMM_CH);
	HAL_StatusTypeDef st;
	if ((ret == HAL_UART_STATE_READY ||
			ret == HAL_UART_STATE_BUSY_RX) == false) {
		// UART is not ready to transmit
		UART_LOG_ERROR(COMP, "UART not ready : 0x%X", ret);
		return false;
	}
	message->head.target_address = 0x00; // sent it to master
	message->head.sender_address = UBA_6_device_g.settings.address;
	message->which_pyload = tag;
	status = pb_get_encoded_size(&message_size, &MSG_Message_msg, message);
	if (status) {
		UART_LOG_COMM_DEBUG("Encoded size: %u bytes", message_size);
	} else {
		UART_LOG_COMM_DEBUG("Error getting encoded size");
		return false;
	}
	index = encode_varint(message_size, uart_tx_dma_buffer);
	UART_LOG_COMM_DEBUG("varint size: %u bytes", index);
	pb_ostream_t stream = pb_ostream_from_buffer(&uart_tx_dma_buffer[index], sizeof(uart_tx_dma_buffer) - index);
	status = pb_encode(&stream, MSG_Message_fields, message);
	if (!status) {
		UART_LOG_COMM_DEBUG("Encoding failed: %s", PB_GET_ERROR(&stream));
		return false;
	} else {
		UBA_UART_print_buffer(uart_tx_dma_buffer, message_size + index);
		//process_uart_data(uart_tx_dma_buffer, message_size + index);
		st = HAL_UART_Transmit_DMA(&COMM_CH, uart_tx_dma_buffer, message_size + index);
		if (st != HAL_OK) {
			UART_LOG_ERROR(COMP, "TX failed:%u", st);
			return false;
		} else {
			UART_LOG_COMM(COMP, "TX sent: %u = %u+%u", message_size + index, message_size, index);
			//UBA_util_print_buffer(buffer, stream.bytes_written + 1);
			return true;
		}
	}
}
void UBA_UART_query_line(UBA_PROTO_LINE_status *ls, UBA_UART_QUERY_RECIPIENT line_recipient_id) {
	UBA_line *query_line;
	UART_LOG_COMM_INFO("Line Query:%u", line_recipient_id);
	switch (line_recipient_id) {
		case UBA_UART_QUERY_RECIPIENT_LINE_A:
			query_line = &UBA_LINE_A;
			break;
		case UBA_UART_QUERY_RECIPIENT_LINE_B:
			query_line = &UBA_LINE_B;
			break;
		default:
			UART_LOG_ERROR(COMP, "ID is unknoun:%x", line_recipient_id);
			return;
	}
	UBA_line_update_message(query_line, ls);
}
void UBA_UART_query_channel(UBA_PROTO_CHANNEL_status *cs, UBA_UART_QUERY_RECIPIENT ch_recipient_id) {
	UBA_channel *query_ch;
	switch (ch_recipient_id) {
		case UBA_UART_QUERY_RECIPIENT_CHANNEL_A:
			query_ch = &UBA_CH_A;
			break;
		case UBA_UART_QUERY_RECIPIENT_CHANNEL_B:
			query_ch = &UBA_CH_B;
			break;
		case UBA_UART_QUERY_RECIPIENT_CHANNEL_AB:
			query_ch = &UBA_CH_AB;
			break;
		default:
			UART_LOG_ERROR(COMP, "ID is unknoun:%x", ch_recipient_id);
			return;
	}
	UBA_channel_update_message(query_ch, cs);
}

void UBA_UART_query_BPT(UBA_PROTO_BPT_status_message *bpss, UBA_UART_QUERY_RECIPIENT bpt_recipient_id) {
	UBA_BPT *bpt;
	switch (bpt_recipient_id) {
		case UBA_UART_QUERY_RECIPIENT_BPT_A:
			bpt = &UBA_6_device_g.BPT_A;
			break;
		case UBA_UART_QUERY_RECIPIENT_BPT_B:
			bpt = &UBA_6_device_g.BPT_B;
			break;
		case UBA_UART_QUERY_RECIPIENT_BPT_AB:
			bpt = &UBA_6_device_g.BPT_AB;
			break;
		default:
			UART_LOG_ERROR(COMP, "ID is not a known BPT id:%x", bpt_recipient_id);
			return;
	}
	UBA_BPT_update_message(bpt, bpss);
}

int UBA_UART_comm_init()
{
	// Return to receive mode
//	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_SET);
	HAL_UART_Receive_DMA(&COMM_CH, uart_rx_dma_buffer, UART_RX_BUFFER_SIZE);
	__HAL_UART_CLEAR_IDLEFLAG(&COMM_CH);
	__HAL_UART_ENABLE_IT(&COMM_CH, UART_IT_IDLE);
//	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_SET);

	return 0;
}

int process_message(MSG_Message *message) {
	if ((message->head.target_address & UBA_6_device_g.settings.address) != UBA_6_device_g.settings.address) {
		UART_LOG_ERROR(COMP, "this is not my message (0x%08x) target is 0x%08x", UBA_6_device_g.settings.address, message->head.target_address);
		return -1;
	}
	switch (message->which_pyload) {
		case MSG_Message_query_tag:
			UART_LOG_COMM_DEBUG("Query message: recipient:%u", message->pyload.query.recipient);
			UBA_UART_qeury_pending_post(message->pyload.query.recipient, message->head.id);
			break;
		case MSG_Message_cmd_tag:
			UART_LOG_COMM_DEBUG("CMD message");
			UBA_COMMAND_execute(&message->pyload.cmd);
			break;
		case MSG_Message_tr_tag:
			UART_LOG_COMM_DEBUG("TR message: %s Store at Index: %u", message->pyload.tr.tr.name, message->pyload.tr.index);
			memcpy(&TR_file.list[message->pyload.tr.index], &message->pyload.tr.tr, sizeof(message->pyload.tr.tr));
			UBA_TR_print(&TR_file.list[message->pyload.tr.index]);
			UBA_PROTO_save_to_file(UBA_FM_FOLDER_TEST_ROUTINE, UBA_FM_FILE_NAME_TEST_ROUTINE, TR_Test_Routine_File_fields, &TR_file);
			break;
		case MSG_Message_calibration_tag:
			UART_LOG_COMM_DEBUG("Calibration message");
			UBA_line_load_calibration(&UBA_LINE_A, &(message->pyload.calibration.line_calibration[0]));
			file_logger_calibrate_line_store(UBA_LINE_A.calibration.file_name, &(message->pyload.calibration.line_calibration[0]));
			UBA_line_load_calibration(&UBA_LINE_B, &(message->pyload.calibration.line_calibration[1]));
			file_logger_calibrate_line_store(UBA_LINE_B.calibration.file_name, &(message->pyload.calibration.line_calibration[1]));
			break;
		case MSG_Message_device_settings_tag: /* change all the settoeng*/
			break;
		default:
			UART_LOG_ERROR(COMP, "message is unknown :%u", message->which_pyload);
			return -1;

	}
	return 0;

}

uint32_t decode_varint(uint8_t *data, uint16_t len, uint8_t *varint_index) {
	uint32_t result = 0;
	int shift = 0;
	for (*varint_index = 0; *varint_index < len; (*varint_index)++) {
		result |= (data[*varint_index] & UART_RX_VARINT_VALUE_MASK) << shift;
		if ((data[*varint_index] & UART_RX_VARINT_FLAG_MASK) == 0) {
			(*varint_index)++;
			return result;
		}
		shift += 7;
	}
	(*varint_index)++;
	return result;
}

void process_uart_data(uint8_t *data, uint16_t len) {
	uint8_t message_index;
	UART_LOG_COMM_DEBUG("process uart data len:%u", len);
	uint32_t message_size = decode_varint(data, len, &message_index);
	UART_LOG_COMM_DEBUG("Message Size:%u start At index:%u", message_size, message_index);
	pb_istream_t stream = pb_istream_from_buffer(&data[message_index], message_size);
	MSG_Message message = MSG_Message_init_zero;
	if (pb_decode(&stream, MSG_Message_fields, &message)) {
		UART_LOG_COMM_DEBUG("Successfully decoded! (%u)", message_size);
		process_message(&message);
		// Successfully decoded!
	} else {
		UART_LOG_ERROR(COMP, "Decoding failed: %s", PB_GET_ERROR(&stream));
		UBA_util_print_buffer(data, len);
		// Error
	}
}

void UBA_UART_Idle_callback(UART_HandleTypeDef *huart) {
	uint16_t pre_pos, new_pos = UART_RX_BUFFER_SIZE - __HAL_DMA_GET_COUNTER(huart->hdmarx);
	uint16_t data_len;
	uint8_t temp_buff[UART_RX_BUFFER_SIZE];
	uint16_t DMA_counter = __HAL_DMA_GET_COUNTER(huart->hdmarx);

	UART_LOG_COMM_DEBUG("DMA counter:%04u new pos:%04u old pos:%04u", DMA_counter, new_pos, old_pos);
	if (new_pos != old_pos) {
		uint32_t primask = __get_PRIMASK();
		__disable_irq();
		if (new_pos > old_pos) {
			data_len = new_pos - old_pos;
			process_uart_data(&uart_rx_dma_buffer[old_pos], data_len);
		} else {
			// Wrapped around
			data_len = UART_RX_BUFFER_SIZE - old_pos;
			UART_LOG_WARNNING(COMP, "Wrapped around rx Data :%u new pos:%u", data_len, new_pos);
			memcpy(temp_buff, &uart_rx_dma_buffer[old_pos], data_len);
//			process_uart_data(&uart_rx_dma_buffer[old_pos], data_len);
			if (new_pos > 0) {
//				process_uart_data(&uart_rx_dma_buffer[0], new_pos);
				memcpy(&temp_buff[data_len], &uart_rx_dma_buffer[0], new_pos);
			}
			process_uart_data(temp_buff, new_pos + data_len);
		}
		old_pos = new_pos;
		if (old_pos == UART_RX_BUFFER_SIZE) {
			old_pos = 0;
		}
		if (!primask)
			__enable_irq();

	} else {
		// Stop DMA safely
		memset(uart_rx_dma_buffer, 0, UART_RX_BUFFER_SIZE);
		HAL_UART_DMAStop(huart);

		// Clear UART status flags
		__HAL_UART_CLEAR_OREFLAG(huart);
		__HAL_UART_CLEAR_IDLEFLAG(huart);

		// Restart DMA reception
		HAL_StatusTypeDef status_ret = HAL_UART_Receive_DMA(huart, uart_rx_dma_buffer, UART_RX_BUFFER_SIZE);

		// Re-enable IDLE interrupt
		__HAL_UART_ENABLE_IT(huart, UART_IT_IDLE);

//		HAL_StatusTypeDef status_ret =HAL_UART_Receive_DMA(huart, uart_rx_dma_buffer, UART_RX_BUFFER_SIZE);
		if (status_ret != HAL_OK) {
			UART_LOG_ERROR(COMP, "Failed to recive to DMA:%04u", status_ret);
		}
		old_pos = 0;
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart->Instance == COMM_CH.Instance) {
	}
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart->Instance == COMM_CH.Instance) {
		DMA_done = true;
	}
}

void UBA_UART_comm_run() {
	HAL_UART_StateTypeDef state;
	if (query_pending_reqest) {
		UBA_UART_query_response_message(query_pending_reqest);
	}
	if (DMA_done) {
		DMA_done = false;
		UART_LOG_COMM_DEBUG("Successfully Sent Message!");
	}
	state = HAL_UART_GetState(&COMM_CH);
	if ((state == HAL_UART_STATE_BUSY_RX || state == HAL_UART_STATE_BUSY_TX_RX) == false) {
		//UART_LOG_WARNNING(COMP, "USART state is %lx", state);
		//HAL_UART_Receive_DMA(&COMM_CH, uart_rx_dma_buffer, UART_RX_BUFFER_SIZE);
	}

}
void UBA_UART_query_device(UBA_PROTO_UBA6_status *status_updated) {
	UBA_6_update_message(&UBA_6_device_g, status_updated);
}

void UBA_UART_query_response_message(UBA_UART_QUERY_RECIPIENT id) {
	MSG_Message message = { 0 };
	UBA_UART_QUERY_RECIPIENT clear_id = UBA_UART_QUERY_RECIPIENT_NONE;
	if (id == UBA_UART_QUERY_RECIPIENT_NONE) {
		UART_LOG_WARNNING(COMP, "there is no :%u", id);
	} else if ((id & UBA_UART_QUERY_RECIPIENT_DEVICE) == UBA_UART_QUERY_RECIPIENT_DEVICE) {
		clear_id = UBA_UART_QUERY_RECIPIENT_DEVICE;
		UBA_UART_query_device(&message.pyload.query_response.status.device);
		message.pyload.query_response.which_status = UBA_PROTO_QUERY_query_response_message_device_tag;
	} else if ((id & UBA_UART_QUERY_RECIPIENT_LINE_A) == UBA_UART_QUERY_RECIPIENT_LINE_A) {
		clear_id = UBA_UART_QUERY_RECIPIENT_LINE_A;
		UBA_UART_query_line(&message.pyload.query_response.status.line, clear_id);
		message.pyload.query_response.which_status = UBA_PROTO_QUERY_query_response_message_line_tag;
	} else if ((id & UBA_UART_QUERY_RECIPIENT_LINE_B) == UBA_UART_QUERY_RECIPIENT_LINE_B) {
		clear_id = UBA_UART_QUERY_RECIPIENT_LINE_B;
		UBA_UART_query_line(&message.pyload.query_response.status.line, clear_id);
		message.pyload.query_response.which_status = UBA_PROTO_QUERY_query_response_message_line_tag;
	} else if ((id & UBA_UART_QUERY_RECIPIENT_CHANNEL_A) == UBA_UART_QUERY_RECIPIENT_CHANNEL_A) {
		clear_id = UBA_UART_QUERY_RECIPIENT_CHANNEL_A;
		UBA_UART_query_channel(&message.pyload.query_response.status.channel, clear_id);
		message.pyload.query_response.which_status = UBA_PROTO_QUERY_query_response_message_channel_tag;
	} else if ((id & UBA_UART_QUERY_RECIPIENT_CHANNEL_B) == UBA_UART_QUERY_RECIPIENT_CHANNEL_B) {
		clear_id = UBA_UART_QUERY_RECIPIENT_CHANNEL_B;
		UBA_UART_query_channel(&message.pyload.query_response.status.channel, clear_id);
		message.pyload.query_response.which_status = UBA_PROTO_QUERY_query_response_message_channel_tag;
	} else if ((id & UBA_UART_QUERY_RECIPIENT_CHANNEL_AB) == UBA_UART_QUERY_RECIPIENT_CHANNEL_AB) {
		clear_id = UBA_UART_QUERY_RECIPIENT_CHANNEL_AB;
		UBA_UART_query_channel(&message.pyload.query_response.status.channel, clear_id);
		message.pyload.query_response.which_status = UBA_PROTO_QUERY_query_response_message_channel_tag;
	} else if ((id & UBA_UART_QUERY_RECIPIENT_BPT_A) == UBA_UART_QUERY_RECIPIENT_BPT_A) {
		clear_id = UBA_UART_QUERY_RECIPIENT_BPT_A;
		UBA_UART_query_BPT(&message.pyload.query_response.status.bpt, clear_id);
		message.pyload.query_response.which_status = UBA_PROTO_QUERY_query_response_message_bpt_tag;
	} else if ((id & UBA_UART_QUERY_RECIPIENT_BPT_B) == UBA_UART_QUERY_RECIPIENT_BPT_B) {
		clear_id = UBA_UART_QUERY_RECIPIENT_BPT_B;
		UBA_UART_query_BPT(&message.pyload.query_response.status.bpt, clear_id);
		message.pyload.query_response.which_status = UBA_PROTO_QUERY_query_response_message_bpt_tag;
	} else if ((id & UBA_UART_QUERY_RECIPIENT_BPT_AB) == UBA_UART_QUERY_RECIPIENT_BPT_AB) {
		clear_id = UBA_UART_QUERY_RECIPIENT_BPT_AB;
		UBA_UART_query_BPT(&message.pyload.query_response.status.bpt, clear_id);
		message.pyload.query_response.which_status = UBA_PROTO_QUERY_query_response_message_bpt_tag;
	}
	if (clear_id != UBA_UART_QUERY_RECIPIENT_NONE) {
		message.pyload.query_response.recipient = clear_id;
		message.pyload.query_response.response_id = query_pending_id;
		if (message.pyload.query_response.response_id == 0) {
			message.pyload.query_response.response_id = 0xffffffff;
		}
		if (UBA_UART_sent_message(&message, MSG_Message_query_response_tag)) {
			UART_LOG_COMM_INFO("Sent Response for :%u message ID", query_pending_id);
			UBA_UART_qeury_pending_clear(clear_id);
		}
	}
}

void UBA_UART_transfer_file(char *folder, char *filename, uint32_t chunk_index) {
	MSG_Message message = { 0 };
	message.pyload.file.chunk_index = chunk_index;
	memcpy(message.pyload.file.filename, filename, strlen(filename) + 1);
	UBA_FM_transfer_chunk(folder, &message.pyload.file);
	if (UBA_UART_sent_message(&message, MSG_Message_file_tag)) {
		UART_LOG_WARNNING(COMP, "File: %s Was Sent chunk %lu", message.pyload.file.filename, chunk_index);
	}
}

void UBA_UART_transfer_file_list(char *folder, uint16_t skip) {
	MSG_Message message = { 0 };
	UBA_FM_file_list(folder, &message.pyload.fm_list, skip);
	if (UBA_UART_sent_message(&message, MSG_Message_fm_list_tag)) {
		UART_LOG_WARNNING(COMP, "File List (%u/%u): Was Sent ", message.pyload.fm_list.filenames_count, message.pyload.fm_list.total_files);
	}
}

void UBA_UART_transfer_single_file_name(uint8_t *filename) {
	MSG_Message message = { 0 };
	memcpy(&message.pyload.fm_list.filenames[0][0], filename, strlen(filename) + 1);
	message.pyload.fm_list.filenames_count = 1;
	if (UBA_UART_sent_message(&message, MSG_Message_fm_list_tag)) {
		UART_LOG_WARNNING(COMP, "File List (%u/%u): Was Sent ", message.pyload.fm_list.filenames_count, message.pyload.fm_list.total_files);
	}

}

