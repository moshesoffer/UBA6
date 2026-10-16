/*
 * UBA_file_manager.h
 *
 *  Created on: Aug 17, 2025
 *      Author: ORA
 */

#ifndef UBA_FILE_MANAGER_H_
#define UBA_FILE_MANAGER_H_
#include "stddef.h"
#include "stdint.h"
#include "stdbool.h"
#include "UBA_PROTO_FM.pb.h"

#define UBA_FM_FOLDER_CALIBRATION "/Calibrations"
#define UBA_FM_FOLDER_SETTINGS "/Settings"
#define UBA_FM_FOLDER_TEST_RESULTS "/Test Results"
#define UBA_FM_FOLDER_TEST_ROUTINE "/Test Routine"
#define UBA_FM_FILE_NAME_SETTINGS "UBA Settings.pb"
#define UBA_FM_FILE_NAME_TEST_ROUTINE "Test Routine.pb"
#define UBA_FM_FILE_NAME_CALIBRATION "Calibration.pb"


bool UBA_FM_create_file(char *folder, char *file_name) ;
bool UBA_FM_apppned_data(char* folder,char* file_name,uint8_t * data, uint32_t data_length);
bool UBA_FM_store_data(char* folder,char* file_name,uint8_t * data, uint32_t data_length); // create file then / overwrite
int UBA_FM_read_data(char* folder,char* file_name,uint8_t * data_out, uint32_t max_data_length);
int UBA_FM_seek_read_data(char *folder, char *file_name, uint32_t seek, uint8_t *data_out, uint32_t max_data_length);
int UBA_FM_read_chunk(char* folder,char* file_name,uint8_t * data, uint32_t chank_size,uint32_t chunk_number); // read offset from file advance by chunk   size
void UBA_FM_command_execute(UBA_PROTO_FM_command * cmd);


uint32_t UBA_FM_file_size(char* folder,char* file_name);
int UBA_FM_transfer_chunk(char* folder,UBA_PROTO_FM_file_transfer* msg);
void UBA_FM_file_list(char *folder, UBA_PROTO_FM_file_list *list, uint16_t skip);

#endif /* UBA_FILE_MANAGER_H_ */
