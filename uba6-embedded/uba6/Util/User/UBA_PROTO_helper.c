/*
 * UBA_PROTO_helper.c
 *
 *  Created on: Aug 24, 2025
 *      Author: ORA
 */

#include "UBA_PROTO_helper.h"
#include "pb.h"
#include "pb_decode.h"
#include "pb_encode.h"
#include "UBA_PROTO_BPT.pb.h"
#include "UBA_PROTO_CALIBRATION.pb.h"
#include "UBA_PROTO_CHANNEL.pb.h"
#include "UBA_PROTO_CMD.pb.h"
#include "UBA_PROTO_DATA_LOG.pb.h"
#include "UBA_PROTO_LINE.pb.h"
#include "UBA_PROTO_QUERY.pb.h"
#include "UBA_PROTO_TR.pb.h"
#include "UBA_PROTO_UBA6.pb.h"
#include "uart_log.h"
#include "UBA_file_manager.h"
#include "UBA_util.h"


#define UART_PROTO_VARINT_SHIFT_SIZE (7)
#define UART_PROTO_VARINT_FLAG_MASK (1<<UART_PROTO_VARINT_SHIFT_SIZE)
#define UART_PROTO_VARINT_VALUE_MASK (UART_PROTO_VARINT_FLAG_MASK -1 )
#define UART_PROTO_VARINT_MAX_BYTE_SIZE (UART_PROTO_VARINT_VALUE_MASK)
#define UART_PROTO_HELPER_MAX_FILE_SIZE (TR_Test_Routine_File_size)
static uint8_t file_buf[UART_PROTO_HELPER_MAX_FILE_SIZE] = { 0 };

#define COMP "PROTO helper"

#if (UBA_LOG_LEVEL_PROTO_H <= UART_LOG_LEVEL_INFO)
	#define UART_LOG_COMP_INFO(...) UART_LOG_INFO(COMP,##__VA_ARGS__)
#else
	#define UART_LOG_COMP_INFO(...)
#endif

#if UBA_LOG_LEVEL_PROTO_H <= UART_LOG_LEVEL_DEBUG
	#define UART_LOG_COMP_DEBUG(...)  UART_LOG_DEBUG(COMP,##__VA_ARGS__)
#else
	#define UART_LOG_COMP_DEBUG(...)
#endif

size_t UBA_PROTO_helper_encode_varint(uint32_t value, uint8_t *output) {
	size_t i = 0;
	while (value > UART_PROTO_VARINT_MAX_BYTE_SIZE) {
		output[i++] = (value & UART_PROTO_VARINT_VALUE_MASK) | UART_PROTO_VARINT_FLAG_MASK;
		value >>= UART_PROTO_VARINT_SHIFT_SIZE;
	}
	output[i++] = value;

	return i;
}

uint32_t UBA_PROTO_helper_decode_varint(uint8_t *data, uint16_t len, uint8_t *varint_index) {
	uint32_t result = 0;
	int shift = 0;
	for (*varint_index = 0; *varint_index < len; (*varint_index)++) {
		result |= (data[*varint_index] & UART_PROTO_VARINT_VALUE_MASK) << shift;
		if ((data[*varint_index] & UART_PROTO_VARINT_FLAG_MASK) == 0) {
			(*varint_index)++;
			return result;
		}
		shift += UART_PROTO_VARINT_SHIFT_SIZE;
	}
	(*varint_index)++;
	return result;
}

void print_data_log(UBA_PROTO_DATA_LOG_data_log *msg) {
	UART_LOG_COMP_DEBUG("Log Data :%015lu step index :%02u plan index:%02u Data: %07d mA %07d mV %07d C/100",
			msg->time,
			msg->step_index,
			msg->plan_index,
			msg->current,
			msg->voltage,
			msg->temp
			);
}

void print_data_log_file(char *folder, char *filename) {
	uint8_t data_buf[UBA_PROTO_DATA_LOG_data_log_size + 1];
	uint32_t seek = 0;
	uint32_t data_length = 1;
	uint32_t read_length = 0;
	uint8_t varint_index = 0;
	UBA_PROTO_DATA_LOG_data_log msg = UBA_PROTO_DATA_LOG_data_log_init_zero;
	pb_istream_t stream;
	do {
		data_length = 1; // read varint
		read_length = UBA_FM_seek_read_data(folder, filename, seek, data_buf, data_length);
		if (read_length == data_length) {
			seek += read_length;
			data_length = UBA_PROTO_helper_decode_varint(data_buf, read_length, &varint_index);
		} else {
			return;
		}
		read_length = UBA_FM_seek_read_data(folder, filename, seek, data_buf, data_length);
		if (read_length == data_length) {
			seek += read_length;
		} else {
			return;
		}
		stream = pb_istream_from_buffer(data_buf, read_length);
		if (pb_decode(&stream, UBA_PROTO_DATA_LOG_data_log_fields, &msg)) {
			print_data_log(&msg);
		} else {
			UART_LOG_ERROR(COMP, "Failed To decode message");
		}
	} while (seek > 0);

}

bool UBA_PROTO_load_from_file(char *folder, char *filename, const pb_msgdesc_t *fields, void *src_struct) {
	bool ret = false;
	pb_istream_t stream;
	size_t read_size = UBA_FM_read_data(folder, filename, file_buf, UART_PROTO_HELPER_MAX_FILE_SIZE);
	if (read_size > 0) {
		stream = pb_istream_from_buffer(file_buf, read_size);
		if (pb_decode(&stream, fields, src_struct)) {
			ret = true;
		} else {
			UART_LOG_ERROR(COMP, "Failed To decode message : %s", stream.errmsg);
			ret = false;
		}
	}
	else {
		UART_LOG_ERROR(COMP, "Failed Read File: %s/%s", folder, filename);
	}
	return ret;
}

bool UBA_PROTO_save_to_file(char *folder, char *filename, const pb_msgdesc_t *fields, const void *src_struct) {
	bool ret = false;
	pb_ostream_t stream;
	bool status;
	stream = pb_ostream_from_buffer(file_buf, UART_PROTO_HELPER_MAX_FILE_SIZE);
	status = pb_encode(&stream, fields, src_struct);
	if (status) {
		UART_LOG_DEBUG(COMP, "Encoded size: %u bytes", stream.bytes_written);
		ret = UBA_FM_store_data(folder, filename, file_buf, stream.bytes_written);
	} else {
		UART_LOG_ERROR(COMP, "Error getting encoded : %s ", stream.errmsg);
		ret = false;
	}
	if (ret == false) {
		UART_LOG_ERROR(COMP, "Save message To file Failed");
	}
	return ret;
}

