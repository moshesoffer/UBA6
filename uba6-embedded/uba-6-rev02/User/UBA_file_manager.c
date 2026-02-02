/*
 * UBA_file_manager.c
 *
 *  Created on: Aug 17, 2025
 *      Author: ORA
 */
#include "UBA_file_manager.h"
#include "ff.h"
#include "diskio.h"
#include "string.h"
#include <stdio.h>
#include "uart_log.h"
#include "UBA_util.h"
#include "UBA_UART_comm.h"
#include "UBA_battery_performance_test.h"
#include "UBA_6.h"

#define COMPONENT "File Manger"
static FRESULT res;
static FILINFO fno;
static FIL file;
static DIR dir;
static FATFS fs;
static UINT bw;

#define MAX_FILES   20   // max number of files to collect
#define MAX_NAME    64    // max file name length

static bool is_SD_card_mount = false;

#define UBA_FM_SD_CARD (1)
#define UBA_FM_MAX_FILE_NAMEPATH (128)
static char filepath[UBA_FM_MAX_FILE_NAMEPATH];

#if (UBA_LOG_LEVEL_FILE_MAN <= UART_LOG_LEVEL_INFO)
#define UART_FM_INFO(...) UART_LOG_INFO(COMPONENT,##__VA_ARGS__)
#else
#define UART_FM_INFO(...)
#endif

#if (UBA_LOG_LEVEL_FILE_MAN <= UART_LOG_LEVEL_DEBUG)
#define UART_FM_DEBUG(...)  UART_LOG_DEBUG(COMPONENT,##__VA_ARGS__)
#else
#define UART_FM_DEBUG(...)
#endif

#if UBA_LOG_LEVEL_FILE_MAN <= UART_LOG_LEVEL_ERROR
#define UART_FM_ERROR(...)  UART_LOG_ERROR(COMPONENT,##__VA_ARGS__)
#else
#define UART_FM_ERROR(...)
#endif
#define UBA_FM_SD_CARD_MOUNT_OPT (1)

bool UBA_FM_SD_mount(void) {
	bool ret = is_SD_card_mount;
	if (is_SD_card_mount == false) {
		res = f_mount(&fs, "", UBA_FM_SD_CARD_MOUNT_OPT);
		ret = (res == FR_OK);
		if (ret) {
			UART_FM_DEBUG("Mount SD Card");
			is_SD_card_mount = true;
		} else {
			UART_FM_ERROR("Failed To Mount SD Card");
		}
	}
	return ret;
}

bool UBA_FM_SD_unmount(void) {
	bool ret = !is_SD_card_mount;
	if (is_SD_card_mount) {
		res = f_mount(NULL, "", UBA_FM_SD_CARD_MOUNT_OPT);
		ret = (res == FR_OK);
		if (ret) {
			UART_FM_DEBUG("UN-Mount SD Card");
			is_SD_card_mount = false;
		} else {
			UART_FM_ERROR("Failed To Un-Mount SD Card");
		}
	}
	return ret;
}

bool UBA_FM_FF_is_file_exist(char *filepath_str) {
	bool ret = false;
	res = f_open(&file, filepath, FA_OPEN_EXISTING | FA_READ);
	if (res == FR_NO_FILE) {
		UART_FM_DEBUG("File :%s dose not exist", filepath_str);
	} else if (res == FR_OK) {
		f_close(&file);
		ret = true;
		UART_FM_DEBUG("File :%s exist", filepath_str);
	} else {
		UART_FM_DEBUG("File :%s dose not exist : 0x%02X", filepath_str, res);
	}
	return ret;
}

bool UBA_FM_create_folder(char *folder_name) {
	bool ret = false;
	if (UBA_FM_SD_mount()) {
		if (f_opendir(&dir, folder_name) != FR_OK) {
			// Folder does not exist, create it
			UART_FM_DEBUG("Folder :%s does not exist, create it ", folder_name);
			res = f_mkdir(folder_name);
			if (res != FR_OK) {
				UART_FM_ERROR("Failed To Create Folder:%s", folder_name);
			}
		} else {
			UART_FM_DEBUG("Folder:%s already exists ", folder_name);
			f_closedir(&dir); // Folder already exists
			ret = true;
		}
	}
	return ret;
}

bool UBA_FM_FF_create_folder(char *folder) {
	res = f_opendir(&dir, folder);
	if (res != FR_OK) {
		UART_FM_DEBUG("Folder : %s  does not exist (0x%0x), create it", folder, res);
		res = f_mkdir(folder);
		if (res != FR_OK) {
			UART_FM_ERROR("Failed to create Folder:%s 0x%02x", folder, res);
			return false;
		}
	} else {
		UART_FM_DEBUG("Folder: %s already exists", folder);
		f_closedir(&dir); //
	}
	return true;
}

bool UBA_FM_create_floder(char *folder) {
	bool ret = false;
	if (UBA_FM_SD_mount()) {
		ret = UBA_FM_FF_create_folder(folder);
		UBA_FM_SD_unmount();
	}
	return ret;
}
bool create_filepath(char *folder, char *file_name, char *filepath_string, uint32_t max_length) {
	if (strlen(folder) + strlen(file_name) > (max_length - 1)) {
		UART_FM_ERROR("Pathname:%s/%s is to long (>%u)", folder, file_name, max_length);
		return false;
	} else {
		memset(filepath, 0, max_length);
		snprintf(filepath, max_length, "%s/%s", folder, file_name);
		UART_FM_DEBUG("create new filepath: %s", filepath_string);
		return true;
	}
}

bool UBA_FM_create_file(char *folder, char *file_name) {
	bool ret = false;
	if (UBA_FM_SD_mount()) {
		create_filepath(folder, file_name, filepath, UBA_FM_MAX_FILE_NAMEPATH);
		if (UBA_FM_FF_create_folder(folder)) {
			if (UBA_FM_FF_is_file_exist(filepath)) {
				ret = true;
			} else {
				res = f_open(&file, filepath, FA_CREATE_NEW | FA_WRITE);
				if (res == FR_OK) {
					UART_FM_DEBUG(" File:%s already exists", filepath);
					f_close(&file);
					ret = true;
				} else if (res == FR_EXIST) {
					UART_FM_DEBUG(" File:%s already exists", filepath);
					ret = true;
				} else {
					UART_FM_ERROR("Failed to create the file:%s", filepath);
					ret = false;
				}
			}
		} else {
			UART_FM_ERROR("Failed to create the folder:%s", folder);
		}
		UBA_FM_SD_unmount();
	}
	return ret;
}

bool UBA_FM_apppned_data(char *folder, char *file_name, uint8_t *data, uint32_t data_length) {
	bool ret = false;
	if (UBA_FM_SD_mount()) {
		if (create_filepath(folder, file_name, filepath, UBA_FM_MAX_FILE_NAMEPATH) && UBA_FM_FF_create_folder(folder)) {
			res = f_open(&file, (char*) filepath, FA_OPEN_APPEND | FA_WRITE);
			if (res == FR_OK) {
				res = f_write(&file, data, data_length, &bw); // Write at the end of the file
				if (res == FR_OK && bw == data_length) {
					UART_FM_DEBUG("Successfully Append to file %s Data size : %lu", filepath, data_length);
				} else {
					UART_FM_ERROR("Failed to Append to file:%s - 0x%02x", filepath, res);
				}
				f_close(&file);
			} else if (res == FR_NO_PATH) {
				UART_FM_ERROR("Failed to Append to file:%s - 0x%02x - Path not found", filepath, res);
			} else {
				UART_FM_ERROR("Failed to Append to file:%s - 0x%02x", filepath, res);
				// Handle open error
			}
		}
		UBA_FM_SD_unmount();
	}
	return ret;
}
bool UBA_FM_store_data(char *folder, char *file_name, uint8_t *data, uint32_t data_length) {
	bool ret = false;
	if (UBA_FM_SD_mount()) {
		if (create_filepath(folder, file_name, filepath, UBA_FM_MAX_FILE_NAMEPATH) && UBA_FM_FF_create_folder(folder)) {
			res = f_open(&file, filepath, FA_CREATE_ALWAYS | FA_WRITE); // Create (or overwrite) the file
			if (res == FR_OK) {
				res = f_write(&file, data, data_length, &bw);
				if (res == FR_OK && bw == data_length) {
					// Successfully written
					UART_FM_DEBUG("Successfully Sore To File :%s ", filepath);
					ret = true;
				} else {
					UART_FM_ERROR("Failed to Store data  to file:%s - 0x%02x", filepath, res);
				}
				f_close(&file);
			}
		}
		UBA_FM_SD_unmount();
	}
	return ret;
}
int UBA_FM_read_data(char *folder, char *file_name, uint8_t *data_out, uint32_t max_data_length) {
	bw = 0;
	if (UBA_FM_SD_mount()) {
		if (create_filepath(folder, file_name, filepath, UBA_FM_MAX_FILE_NAMEPATH)) {
			res = f_open(&file, filepath, FA_READ); // Create (or overwrite) the file
			if (res == FR_OK) {
				res = f_read(&file, data_out, max_data_length, &bw);
				if (res == FR_OK) {
					// Successfully read
					UART_FM_DEBUG("Successfully Read %lu Bytes form %s", bw, filepath);
				} else {
					UART_FM_ERROR("Failed to read data from file:%s - 0x%02x", filepath, res);
				}
				f_close(&file);
			} else {
				UART_FM_ERROR("Failed to read data from file:%s - 0x%02x", filepath, res);
			}
		}
		UBA_FM_SD_unmount();
	}
	return bw;
}
int UBA_FM_seek_read_data(char *folder, char *file_name, uint32_t seek, uint8_t *data_out, uint32_t max_data_length) {
	bw = 0;
	if (UBA_FM_SD_mount()) {
		if (create_filepath(folder, file_name, filepath, UBA_FM_MAX_FILE_NAMEPATH)) {
			res = f_open(&file, filepath, FA_READ); // Create (or overwrite) the file
			if (res == FR_OK) {
				res = f_lseek(&file, seek); // move seek to the correct position
				if (res == FR_OK) {
					res = f_read(&file, data_out, max_data_length, &bw);
					if (res == FR_OK) {
						// Successfully read
						UART_FM_DEBUG("Successfully Read %lu Bytes form %s", bw, filepath);
					} else {
						UART_FM_ERROR("Failed to read data from file:%s - 0x%02x", filepath, res);
					}
				} else {
					UART_FM_ERROR("Failed to move seek to new pos file:%s - 0x%02x", filepath, res);
				}
				f_close(&file);
			} else {
				UART_FM_ERROR("Failed to read data from file:%s - 0x%02x", filepath, res);
			}
		}
		UBA_FM_SD_unmount();
	}
	return bw;
}
int UBA_FM_read_chunk(char *folder, char *file_name, uint8_t *data, uint32_t chank_size, uint32_t chunk_number) {
	uint32_t new_seek = chank_size * chunk_number;
	return UBA_FM_seek_read_data(folder, file_name, new_seek, data, chank_size);
}

void UBA_FM_file_list(char *folder, UBA_PROTO_FM_file_list *list, uint16_t skip) {
	list->filenames_count = 0;
	if (UBA_FM_SD_mount()) {
		res = f_opendir(&dir, folder);
		if (res != FR_OK) {
			UART_FM_ERROR("Failed to open folder:%s - 0x%02x", folder, res);
		} else {
			for (;;) {
				res = f_readdir(&dir, &fno);
				if (res != FR_OK || fno.fname[0] == 0)
					break;

				if (!(fno.fattrib & AM_DIR)) { // only files, skip folders
					list->total_files++;
					if (skip) {
						skip--;
					} else if (list->filenames_count < MAX_FILES) {
						strncpy(list->filenames[list->filenames_count], fno.fname, MAX_NAME - 1);
						list->filenames[list->filenames_count][MAX_NAME - 1] = '\0'; // null-terminate
						list->filenames_count++;
					}
				}
			}
			f_closedir(&dir);
		}
		UBA_FM_SD_unmount();
	}
}

void UBA_FM_command_execute(UBA_PROTO_FM_command *cmd) {
	UART_FM_DEBUG("Command Execute");
	switch (cmd->id) {
		case UBA_PROTO_FM_CMD_ID_TEST:
			UART_FM_DEBUG("Test Test");
			break;
		case UBA_PROTO_FM_CMD_ID_CHUNK_REQUEST:
			UART_FM_DEBUG("Transfer File: %s", cmd->filename);
			UBA_UART_transfer_file(UBA_FM_FOLDER_TEST_RESULTS, cmd->filename, cmd->chunk_index);
			break;
		case UBA_PROTO_FM_CMD_ID_FILE_LIST_REQUEST:
			UART_FM_DEBUG("Transfer File List");
			UBA_UART_transfer_file_list(UBA_FM_FOLDER_TEST_RESULTS, cmd->chunk_index);
			break;
		case UBA_PROTO_FM_CMD_ID_BPT_FILE:
			UART_FM_DEBUG("Transfer BPT File Name");
			UBA_BPT *t ;
			if(cmd->chunk_index == UBA_PROTO_CHANNEL_ID_A){
				t = &UBA_6_device_g.BPT_A;
			}else if(cmd->chunk_index == UBA_PROTO_CHANNEL_ID_B){
				t = &UBA_6_device_g.BPT_B;

			}else if (cmd->chunk_index == UBA_PROTO_CHANNEL_ID_AB){
				t = &UBA_6_device_g.BPT_AB;
			}else{
				UART_FM_ERROR("Failed To Detect PBT channel index");
				return;
			}
			UBA_UART_transfer_single_file_name(t->filename);
			break;
		default:
			UART_FM_ERROR("Unknoun Command %0x02X", cmd->id);
			break;

	}
}

uint32_t UBA_FM_file_size(char *folder, char *file_name) {
	uint32_t file_size = 0;
	if (UBA_FM_SD_mount()) {
		if (create_filepath(folder, file_name, filepath, UBA_FM_MAX_FILE_NAMEPATH)) {
			res = f_open(&file, filepath, FA_READ); // Create (or overwrite) the file
			if (res == FR_OK) {
				file_size = f_size(&file);
				UART_FM_DEBUG("File:%s Size:%lu", filepath, file_size);
				f_close(&file);
			} else {
				UART_FM_ERROR("Failed to read data from file:%s - 0x%02x", filepath, res);
			}
		}
		UBA_FM_SD_unmount();
	}
	return file_size;
}

int UBA_FM_transfer_chunk(char *folder, UBA_PROTO_FM_file_transfer *msg) {
	int read = UBA_FM_read_chunk(UBA_FM_FOLDER_TEST_RESULTS, msg->filename, msg->data.bytes, sizeof(msg->data.bytes), msg->chunk_index);
	msg->data.size = read; // the number if bytes read
	msg->total_size = UBA_FM_file_size(folder, msg->filename);
	return read;
}
/*==============================Private functions=====================================================================*/
void UBA_FA_print_tree_dir(const char *path, int depth){
	char buffer[128];
	res = f_opendir(&dir, path);
	if (res == FR_OK) {

		f_closedir(&dir);
	}

}
/*==============================public functions=====================================================================*/
void UBA_FA_print_tree(const char *path){
	if (UBA_FM_SD_mount()) {
		UBA_FA_print_tree_dir(path,0);
		UBA_FM_SD_unmount();
	}
}
