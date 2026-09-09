/*
 * UBA_PROTO_helper.h
 *
 *  Created on: Aug 24, 2025
 *      Author: ORA
 */

#ifndef UBA_PROTO_HELPER_H_
#define UBA_PROTO_HELPER_H_
#include <stddef.h>
#include <stdint.h>
#include "UBA_PROTO_DATA_LOG.pb.h"

size_t UBA_PROTO_helper_encode_varint(uint32_t value, uint8_t *output) ;
uint32_t UBA_PROTO_helper_decode_varint(uint8_t *data, uint16_t len, uint8_t *varint_index);
void print_data_log(UBA_PROTO_DATA_LOG_data_log * msg);
void print_data_log_file(char * folder , char * filename);
bool UBA_PROTO_encode(pb_ostream_t *stream, const pb_msgdesc_t *fields, const void *src_struct);
bool UBA_PROTO_load_from_file(char * folder , char * filename, const pb_msgdesc_t *fields,  void *src_struct);
bool UBA_PROTO_save_to_file(char * folder , char * filename,const pb_msgdesc_t *fields, const void *src_struct);

#endif /* UBA_PROTO_HELPER_H_ */
