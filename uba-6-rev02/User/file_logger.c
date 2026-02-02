/*
 * flie_logger.c
 *
 *  Created on: Aug 19, 2024
 *      Author: ORA
 */
#include <file_logger.h>
#include "ff.h"
#include "diskio.h"
#include <stdio.h>
#include "uart_log.h"
#include "string.h"
#include "UBA_util.h"
#include <pb_encode.h>
#include <pb_decode.h>
#include "UBA_PROTO_CALIBRATION.pb.h"
#include "UBA_file_manager.h"

#define COMPONENT "File Logger"

#if (UBA_LOG_LEVEL_FILE <= UART_LOG_LEVEL_INFO)
#define UART_LOG_FILE_INFO(...) UART_LOG_INFO(COMPONENT,##__VA_ARGS__)
#else
#define UART_LOG_FILE_INFO(...)
#endif

#if UBA_LOG_LEVEL_FILE <= UART_LOG_LEVEL_DEBUG
#define UART_LOG_FILE_DEBUG(...)  UART_LOG_DEBUG(COMPONENT,##__VA_ARGS__)
#else
#define UART_LOG_FILE_DEBUG(...)
#endif

#define MAX_BUFFER_SIZE 256

static void file_logger_print_tree(const char *path, int depth) {
	FRESULT res;
	FILINFO fno;
	DIR dir;
	char buffer[256];

	res = f_opendir(&dir, path); /* Open the directory */
	if (res == FR_OK) {
		for (;;) {
			res = f_readdir(&dir, &fno); /* Read a directory item */
			if (res != FR_OK || fno.fname[0] == 0)
				break; /* Break on error or end of dir */
			if (fno.fattrib & AM_DIR) { /* It is a directory */
				snprintf(buffer, sizeof(buffer), "%s/%s", path, fno.fname);
				UART_LOG_FILE_DEBUG("%*s[%s]", depth * 2, "", fno.fname);
				//UBA_6_log(buffer);
				file_logger_print_tree(buffer, depth + 1); /* Recursively traverse the directory */
			} else { /* It is a file */
				UART_LOG_FILE_DEBUG("%*s%s", depth * 2, "", fno.fname);

			}
		}
		f_closedir(&dir);
	}
}

void create_folder_and_file(const char *folder_name, const char *file_name,
		const char *content) {
	FRESULT res;
	FIL file;
	UINT bw;

	// Create a new folder
	res = f_mkdir(folder_name);
	if (res == FR_OK || res == FR_EXIST) { // FR_EXIST means the folder already exists
		UART_LOG_FILE_INFO("Folder '%s' created or already exists", folder_name);

		// Construct the file path
		char file_path[128];
		snprintf(file_path, sizeof(file_path), "%s/%s", folder_name, file_name);

		// Create and open the file
		res = f_open(&file, file_path, FA_CREATE_ALWAYS | FA_WRITE);
		if (res == FR_OK) {
			// Write content to the file
			res = f_write(&file, content, strlen(content), &bw);
			if (res == FR_OK && bw == strlen(content)) {
				UART_LOG_FILE_INFO("File '%s' created with content: %s", file_path,
						content);
			} else {
				UBA_6_log("Failed to write to file '%s'.\n", file_path);
			}
			f_close(&file);
		} else {
			UBA_6_log("Failed to create file '%s'.\n", file_path);
		}
	} else {
		UBA_6_log("Failed to create folder '%s'.\n", folder_name);
	}
}

void print_file_tree(const char *path, int depth) {
	FRESULT res;
	FILINFO fno;
	DIR dir;
	char buffer[256];

	res = f_opendir(&dir, path); /* Open the directory */
	if (res == FR_OK) {
		for (;;) {
			res = f_readdir(&dir, &fno); /* Read a directory item */
			if (res != FR_OK || fno.fname[0] == 0)
				break; /* Break on error or end of dir */
			if (fno.fattrib & AM_DIR) { /* It is a directory */
				UBA_6_log("%*s[%s]\n", depth * 2, "", fno.fname);
				snprintf(buffer, sizeof(buffer), "%s/%s", path, fno.fname);
				print_file_tree(buffer, depth + 1); /* Recursively traverse the directory */
			} else { /* It is a file */
				UBA_6_log(fno.fname);
			}
		}
		f_closedir(&dir);
	}
}

void file_logger_print(void) {
	FATFS fs;
	UART_LOG_INFO(COMPONENT, "File Logger Print");
	// Mount the filesystem
	if (f_mount(&fs, "", 1) == FR_OK) {
		// Create a folder and file
		create_folder_and_file("/NEW", "new_file.txt", "Hello, world!");

		// Print the file tree
		UART_LOG_DEBUG(COMPONENT, "File tree after creation:");
		file_logger_print_tree("", 0);
	} else {
		UART_LOG_CRITICAL(COMPONENT, "Failed to mount the filesystem");
	}

	// Unmount the filesystem
	f_mount(NULL, "", 0);
}

int flie_logger_overwite(uint8_t *file_name, uint8_t *data, uint8_t data_length) {
	FATFS fs;           // File system object
	FIL file;           // File object
	FRESULT fr;         // FatFs function common result code
	UINT bw;            // File write count
	fr = f_mount(&fs, "", 1);
	if (fr != FR_OK) {
		// Handle mount error
		UART_LOG_CRITICAL(COMPONENT, "Failed to Mount: %02x", fr);
		return -1;
	}
	fr = f_open(&file, file_name, FA_CREATE_ALWAYS | FA_WRITE);
	if (fr != FR_OK) {
		// Handle open error
		UART_LOG_CRITICAL(COMPONENT, "Failed to Open: %02x", fr);
		f_mount(NULL, "", 1); // Unmount on failure
		return -1;
	}
	fr = f_write(&file, data, data_length, &bw);
	if (fr != FR_OK || bw != data_length) {
		// Handle write error
		UART_LOG_CRITICAL(COMPONENT, "Failed to write: %02x", fr);
		f_close(&file);
		f_mount(NULL, "", 1); // Unmount on failure
		return -1;
	}
	f_close(&file);

	// Unmount filesystem
	f_mount(NULL, "", 1);
	return 0;
}

int flie_logger_read(uint8_t *file_name, uint8_t *buffer, uint32_t buffer_length) {
	FATFS fs;
	FIL file;
	FRESULT fr;
	UINT bytesRead;
	fr = f_mount(&fs, "", 1);
	if (fr != FR_OK) {
		// Handle mount error
		UART_LOG_CRITICAL(COMPONENT, "Failed to Mount: %02x", fr);
		return -1;
	}
	fr = f_open(&file, (char*)file_name,  FA_READ);
	if (fr != FR_OK) {
		// Handle open error
		UART_LOG_CRITICAL(COMPONENT, "Failed to Open: %02x", fr);
		f_mount(NULL, "", 1); // Unmount on failure
		return -1;
	}
	// Get file size
	DWORD fileSize = f_size(&file);

	UART_LOG_INFO(COMPONENT,"Open File:%s size [%u]", file_name,fileSize);
	fr = f_read(&file, buffer, buffer_length, &bytesRead);
	if (fr != FR_OK) {
		f_close(&file);
		f_mount(NULL, "", 1);
		UART_LOG_CRITICAL(COMPONENT, "Failed to read file: %02x", fr);
		return -1;
	}
	UART_LOG_INFO(COMPONENT,"Read File:%s [%u] bytes Read", file_name,bytesRead);
	f_close(&file);

	// Unmount filesystem
	f_mount(NULL, "", 1);
	return bytesRead;

}

int file_logger_append(uint8_t *file_name, bpt_data_log *log) {
	FATFS fs;           // File system object
	FIL file;           // File object
	FRESULT fr;         // FatFs function common result code
	UINT bw;            // File write count

	// Mount the filesystem
	fr = f_mount(&fs, "", 1);
	if (fr != FR_OK) {
		// Handle mount error
		UART_LOG_CRITICAL(COMPONENT, "Failed to Mount: %02x", fr);
		return -1;
	}

	// Open or create file for append
	fr = f_open(&file, (char*)file_name, FA_OPEN_APPEND | FA_WRITE);
	if (fr != FR_OK) {
		// Handle open error
		f_mount(NULL, "", 1); // Unmount on failure
		return -1;
	}
	// Prepare CSV line
	char buffer[256] = { 0 };

	snprintf(buffer, sizeof(buffer), "%lu,%u,%d,%d,%.2f\r\n", log->time, log->voltage, log->current, log->cap, log->temp);

	// Write data
	fr = f_write(&file, buffer, strlen(buffer), &bw);
	if (fr != FR_OK || bw != strlen(buffer)) {
		// Handle write error
	}

	// Close the file
	f_close(&file);

	// Unmount filesystem
	f_mount(NULL, "", 1);
	return 0;
}

int file_logger_channel_append(uint8_t *file_name, UBA_PROTO_CHANNEL_data_message *data) {
	bpt_data_log log = { 0 };
	log.time = HAL_GetTick();
	log.voltage = data->capacity;
	log.current = data->current;
	log.cap = data->capacity;
	log.temp = data->temperature;
	return file_logger_append(file_name, &log);
}

int file_logger_calibrate_line_store(uint8_t *file_name, UBA_PROTO_CALIBRATION_line_calibration_message *message) {
	uint8_t buffer[MAX_BUFFER_SIZE] = { 0 };
	bool status;
	pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
	status = pb_encode(&stream, UBA_PROTO_CALIBRATION_line_calibration_message_fields, message);
	return flie_logger_overwite(file_name, buffer, stream.bytes_written);
}






