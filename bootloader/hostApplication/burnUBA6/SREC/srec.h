/*
***************************************************************************
*
* Author: Moshe Soffer
*
***************************************************************************
*/

#ifndef SREC_INCLUDED
#define SREC_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <string.h>

#include <windows.h>

#define ETX_APP_FLASH_ADDR 0x08040000   //Application's Flash Address

extern int num_s3;

int receive_line(FILE *Fptr, char* line_buf, int *line_len);
int extract_srec_line(char* line, int line_len, char* srec, int srec_len);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif


