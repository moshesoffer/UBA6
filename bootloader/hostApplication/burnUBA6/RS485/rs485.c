/*
***************************************************************************
*
* Author: Moshe Soffer
*
***************************************************************************
*/
#include <windows.h>
#include <stdio.h>
#include "rs485.h"

#define RS485_PORTNR  32

HANDLE Cport[RS485_PORTNR];

const char *CportName[RS485_PORTNR]={"\\\\.\\COM1",  "\\\\.\\COM2",  "\\\\.\\COM3",  "\\\\.\\COM4",
                                    "\\\\.\\COM5",  "\\\\.\\COM6",  "\\\\.\\COM7",  "\\\\.\\COM8",
                                    "\\\\.\\COM9",  "\\\\.\\COM10", "\\\\.\\COM11", "\\\\.\\COM12",
                                    "\\\\.\\COM13", "\\\\.\\COM14", "\\\\.\\COM15", "\\\\.\\COM16",
                                    "\\\\.\\COM17", "\\\\.\\COM18", "\\\\.\\COM19", "\\\\.\\COM20",
                                    "\\\\.\\COM21", "\\\\.\\COM22", "\\\\.\\COM23", "\\\\.\\COM24",
                                    "\\\\.\\COM25", "\\\\.\\COM26", "\\\\.\\COM27", "\\\\.\\COM28",
                                    "\\\\.\\COM29", "\\\\.\\COM30", "\\\\.\\COM31", "\\\\.\\COM32"};

HANDLE open_rs485_port(const char* portName, DWORD baudRate);
BOOL read_rs485(HANDLE hSerial, char* buffer, DWORD bufSize, DWORD* bytesRead);
BOOL send_rs485(HANDLE hSerial, const char* data, DWORD length);

int RS485_OpenComport(int comport_number, int baudrate, const char *mode, int flowctrl)
{
  if((comport_number>=RS485_PORTNR)||(comport_number<0))
  {
    printf("illegal comport number\n");
    return(1);
  }

  switch(baudrate)
  {
    case     110 : 
    case     300 :
    case     600 :
    case    1200 :
    case    2400 :
    case    4800 :
    case    9600 :
    case   19200 :
    case   38400 :
    case   57600 : 
    case  115200 : 
    case  128000 : 
    case  256000: break;
    case 1000000: break;

    default      : printf("invalid baudrate\n");
                   return(1);
                   break;
  }

  if(strlen(mode) != 3)
  {
    printf("invalid mode \"%s\"\n", mode);
    return(1);
  }

  switch(mode[0])
  {
    case '8': 
    case '7': 
    case '6': 
    case '5': 
              break;
    default : printf("invalid number of data-bits '%c'\n", mode[0]);
              return(1);
              break;
  }

  switch(mode[1])
  {
    case 'N':
    case 'n': 
    case 'E':
    case 'e': 
    case 'O':
    case 'o': 
              break;
    default : printf("invalid parity '%c'\n", mode[1]);
              return(1);
              break;
  }

  switch(mode[2])
  {
    case '1': 
    case '2': 
              break;
    default : printf("invalid number of stop bits '%c'\n", mode[2]);
              return(1);
              break;
  }

  Cport[comport_number] = open_rs485_port(CportName[comport_number], (DWORD)baudrate);
  if (Cport[comport_number] == INVALID_HANDLE_VALUE)
  {
      printf("unable to open comport\n");
      return(1);
  }

  return(0);
}

int RS485_PollComport(int comport_number, unsigned char* buf, int size)
{
    DWORD bytesRead;

    /* added the void pointer cast, otherwise gcc will complain about */
    /* "warning: dereferencing type-punned pointer will break strict aliasing rules" */

    if (!read_rs485(Cport[comport_number], buf, (DWORD) size, &bytesRead)) 
    {
printf("RS485_PollComport fail\n");
        return -1;
    }

    return(bytesRead);
}

int RS485_SendByte(int comport_number, unsigned char byte)
{
    if (!send_rs485(Cport[comport_number], &byte, (DWORD)1)) 
    {
        CloseHandle(Cport[comport_number]);
        return (1);
    }

    return(0);
}

int RS485_SendBuf(int comport_number, unsigned char* buf, int size)
{
    if (!send_rs485(Cport[comport_number], buf, (DWORD)size)) 
    {
        CloseHandle(Cport[comport_number]);
        return (-1);
    }

    return(size);
}


void RS485_CloseComport(int comport_number)
{
    CloseHandle(Cport[comport_number]);
}


int RS485_FlushComport(int comport_number) {
    // Clear RX + TX buffers
    PurgeComm(Cport[comport_number],
              PURGE_RXCLEAR |
              PURGE_TXCLEAR |
              PURGE_RXABORT |
              PURGE_TXABORT);
    return(0);
}


// Function to configure and open the COM port
HANDLE open_rs485_port(const char* portName, DWORD baudRate) {
    HANDLE hSerial = CreateFileA(
        portName,
        GENERIC_READ | GENERIC_WRITE,
        0,    // exclusive access
        NULL, // default security
        OPEN_EXISTING,
        0,    // no overlapped I/O
        NULL
    );

    if (hSerial == INVALID_HANDLE_VALUE) {
        printf("Error opening port %s (Error %lu)\n", portName, GetLastError());
        return INVALID_HANDLE_VALUE;
    }

    // Configure serial parameters
    DCB dcbSerialParams = { 0 };
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

    if (!GetCommState(hSerial, &dcbSerialParams)) {
        printf("Error getting COM state (Error %lu)\n", GetLastError());
        CloseHandle(hSerial);
        return INVALID_HANDLE_VALUE;
    }

    dcbSerialParams.BaudRate = baudRate;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;

    // Enable RTS for RS485 direction control if needed
    dcbSerialParams.fRtsControl = RTS_CONTROL_ENABLE;

    if (!SetCommState(hSerial, &dcbSerialParams)) {
        printf("Error setting COM state (Error %lu)\n", GetLastError());
        CloseHandle(hSerial);
        return INVALID_HANDLE_VALUE;
    }

    // Set timeouts
    COMMTIMEOUTS timeouts = { 0 };
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 500;
    timeouts.ReadTotalTimeoutMultiplier = 100;
    timeouts.WriteTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;

    if (!SetCommTimeouts(hSerial, &timeouts)) {
        printf("Error setting timeouts (Error %lu)\n", GetLastError());
        CloseHandle(hSerial);
        return INVALID_HANDLE_VALUE;
    }

    return hSerial;
}

// Function to configure the COM port
int RS485_ConfigComport(int comport_number, int read_timeout) {
    // Set timeouts
    COMMTIMEOUTS timeouts = { 0 };
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = (DWORD)read_timeout;
    timeouts.ReadTotalTimeoutMultiplier = 100;
    timeouts.WriteTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;

    if (!SetCommTimeouts(Cport[comport_number], &timeouts)) {
        printf("Error setting timeouts (Error %lu)\n", GetLastError());
        return -1/*INVALID_HANDLE_VALUE*/;
    }
    return 0;
}

// Function to send data
BOOL send_rs485(HANDLE hSerial, const char* data, DWORD length) {
    DWORD bytesWritten;
    if (!WriteFile(hSerial, data, length, &bytesWritten, NULL)) {
        printf("Error writing to port (Error %lu)\n", GetLastError());
        return FALSE;
    }
//    printf("Sent %lu bytes\n", bytesWritten);
    return TRUE;
}

// Function to read data
BOOL read_rs485(HANDLE hSerial, char* buffer, DWORD bufSize, DWORD* bytesRead) {
    if (!ReadFile(hSerial, buffer, bufSize, bytesRead, NULL)) {
        printf("Error reading from port (Error %lu)\n", GetLastError());
        return FALSE;
    }
    return TRUE;
}
