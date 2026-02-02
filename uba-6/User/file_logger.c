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

#define COMPONENT "File Logger"

static void file_logger_print_tree(const char *path, int depth) {
	FRESULT res;
	FILINFO fno;
	DIR dir;
	char buffer[128];

	res = f_opendir(&dir, path); /* Open the directory */
	if (res == FR_OK) {
		for (;;) {
			res = f_readdir(&dir, &fno); /* Read a directory item */
			if (res != FR_OK || fno.fname[0] == 0)
				break; /* Break on error or end of dir */
			if (fno.fattrib & AM_DIR) { /* It is a directory */
				snprintf(buffer, sizeof(buffer), "%s/%s", path, fno.fname);
				 UBA_6_log("%*s[%s]\n", depth * 2, "", fno.fname);
				//UBA_6_log(buffer);
				file_logger_print_tree(buffer, depth + 1); /* Recursively traverse the directory */
			} else { /* It is a file */
				  UBA_6_log("%*s%s\n", depth * 2, "", fno.fname);

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
		UBA_6_log("Folder '%s' created or already exists.\n", folder_name);

		// Construct the file path
		char file_path[128];
		snprintf(file_path, sizeof(file_path), "%s/%s", folder_name, file_name);

		// Create and open the file
		res = f_open(&file, file_path, FA_CREATE_ALWAYS | FA_WRITE);
		if (res == FR_OK) {
			// Write content to the file
			res = f_write(&file, content, strlen(content), &bw);
			if (res == FR_OK && bw == strlen(content)) {
				UBA_6_log("File '%s' created with content: %s\n", file_path,
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
	char buffer[128];

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
		UART_LOG_DEBUG(COMPONENT,"File tree after creation:");
		file_logger_print_tree("", 0);
	} else {
		UART_LOG_CRITICAL(COMPONENT, "Failed to mount the filesystem");
	}

	// Unmount the filesystem
	f_mount(NULL, "", 0);
}
