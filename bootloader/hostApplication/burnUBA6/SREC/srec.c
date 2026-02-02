/*
***************************************************************************
*
* Author: Moshe Soffer
*
***************************************************************************
*/
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <windows.h>

#include "srec.h"

#define LINE_MAX 512  // max SREC line length
#define FLASH_BUF_SIZE 8

static uint8_t hex2byte(const char* s)
{
    uint8_t v = 0;
    for (int i = 0; i < 2; i++) {
        v <<= 4;
        if (s[i] >= '0' && s[i] <= '9') v |= s[i] - '0';
        else if (s[i] >= 'A')           v |= s[i] - 'A' + 10;
    }
    return v;
}

int num_s3 = 0;
int extract_srec_line(char* line, int line_len, char* srec, int srec_len)
{
    static uint8_t flash_buf[FLASH_BUF_SIZE];
    static uint32_t flash_addr = 0;
    static uint8_t buf_idx = 0;

    if (line[0] != 'S')
    {
//        printf("1. (srec_len %d) %c %c\n", srec_len, line[0], line[1]);
        return 0;
    }

    if (line[1] != '3')
    {
//        printf("2. (srec_len %d) %c %c\n", srec_len, line[0], line[1]);
        return 0;   // only S1,2,3 supported
    }

    uint8_t count = hex2byte(&line[2]);
    uint32_t addr =
        (hex2byte(&line[4]) << 24) |
        (hex2byte(&line[6]) << 16) |
        (hex2byte(&line[8]) << 8) |
        hex2byte(&line[10]);

    uint8_t data_len = count - 5;
    uint8_t checksum = count;

    checksum += (addr >> 24) & 0xFF;
    checksum += (addr >> 16) & 0xFF;
    checksum += (addr >> 8) & 0xFF;
    checksum += addr & 0xFF;

    if ((addr - ETX_APP_FLASH_ADDR) > srec_len)
    {
        //printf ("pad with 0xFF: adr %x, srec_len %x, pad %d\n", addr - ETX_APP_FLASH_ADDR, srec_len, ((addr - ETX_APP_FLASH_ADDR) - srec_len));
        //pad with 0xFF
        for (int i = 0; i < ((addr - ETX_APP_FLASH_ADDR) - srec_len); i++)
        {
            srec[srec_len+i] = 0xFF;
        }
        srec_len += ((addr - ETX_APP_FLASH_ADDR) - srec_len);
    }

    uint32_t idx = 12;

    for (int i = 0; i < data_len; i++) {
        uint8_t b = hex2byte(&line[idx]);
        idx += 2;

        checksum += b;

        srec[srec_len++] = b;
//        if (srec_len < 0x300)
//            printf("%02x ", b & 0xff);
    }
//    if (srec_len < 0x300)
//        printf(" (count %d, idx %d, line_len %d)\r\n", count, idx+2, line_len);

    checksum = ~checksum;
    uint8_t file_ck = hex2byte(&line[idx]);

    if (checksum != file_ck) {
        // checksum error → abort OTA
        printf("srec: checksum error\r\n");
        return -1; 
    }
    return srec_len;
}

// Non-blocking or blocking UART RX, example blocking
int receive_line(FILE *Fptr, char* line_buf, int* line_len)
{
    uint32_t idx = 0;
    uint8_t byte;

    char srec_line[256];
    int srec_idx = 0;

    srec_line[0] = '\0';
    fscanf(Fptr, "%s", srec_line);
    if (strlen (srec_line) == 0)
        return -1;
    //printf("%s\n", srec_line);
    while (1) {
        // Receive 1 byte from UART (blocking)
        byte = srec_line[srec_idx++];
        //if (fread(&byte, 1, 1, Fptr) == 0) {
//("end of file fail");
        //    return -1; // receive error
        //}

        if (byte == '\r') continue;  // skip carriage return
        if (byte == ' ')  continue;  // space
        if (byte == '\n') break;     // end of line
        if (byte == '\0') break;     // end of line

        if ((line_buf[idx] == '3') &&
            (line_buf[idx - 1] == 'S'))
            num_s3++;

        if (idx < LINE_MAX - 1) {
            line_buf[idx++] = byte;
        }
        else {
            // line too long ? error
printf("line too long\r\n");
            return 0;
        }
    }

    *line_len = idx+2;
    line_buf[idx] = '\0';  // null-terminate string
    return 1;              // success
}

