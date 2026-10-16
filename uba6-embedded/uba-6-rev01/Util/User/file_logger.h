/*
 * flie_logger.h
 *
 *  Created on: Aug 19, 2024
 *      Author: ORA
 */

#ifndef FILE_LOGGER_H_
#define FILE_LOGGER_H_
#include "stddef.h"
#include "stdint.h"
#include "UBA_PROTO_CHANNEL.pb.h"
#include  "UBA_PROTO_LINE.pb.h"
#include "UBA_PROTO_CALIBRATION.pb.h"

typedef struct {
	uint32_t time;
	float temp;
	uint16_t voltage;
	int16_t current;
	int16_t cap;
}bpt_data_log;


void file_logger_print(void);
int file_logger_append(uint8_t* file_name,bpt_data_log * log);
int file_logger_channel_append(uint8_t* file_name, UBA_PROTO_CHANNEL_data_message* status);
int file_logger_calibrate_line_store(uint8_t* file_name, UBA_PROTO_CALIBRATION_line_calibration_message* message);
int flie_logger_read(uint8_t *file_name, uint8_t *buffer, uint32_t buffer_length) ;

//int file_logger_calibrate_line_retrive(uint8_t* file_name, UBA_PROTO_CALIBRATION_line_calibration_message* message);


#endif /* FILE_LOGGER_H_ */
