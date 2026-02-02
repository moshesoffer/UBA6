/*
***************************************************************************
*
* Author: Moshe Soffer
*
***************************************************************************
*/

#ifndef rs485_INCLUDED
#define rs485_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <string.h>



#if defined(__linux__) || defined(__FreeBSD__)

#include <termios.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <sys/file.h>
#include <errno.h>

#else

#include <windows.h>

#endif

int RS485_OpenComport(int, int, const char *, int);
int RS485_PollComport(int, unsigned char *, int);
int RS485_SendByte(int, unsigned char);
int RS485_SendBuf(int, unsigned char *, int);
void RS485_CloseComport(int);
int RS485_ConfigComport(int, int);
int RS485_FlushComport(int);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif


