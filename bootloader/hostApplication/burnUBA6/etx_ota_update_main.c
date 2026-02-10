
/**************************************************

file: etx_ota_update_main.c
purpose: compile with the command: gcc etx_ota_update_main.c 

**************************************************/
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

#include "rs485.h"
#include "srec.h"
#include "etx_ota_update_main.h"

uint8_t DATA_BUF[ETX_OTA_PACKET_MAX_SIZE];
uint8_t APP_BIN[ETX_OTA_MAX_FW_SIZE];

void delay(uint32_t us)
{
    us *= 10;
#ifdef _WIN32
    //Sleep(ms);
    __int64 time1 = 0, time2 = 0, freq = 0;

    QueryPerformanceCounter((LARGE_INTEGER *) &time1);
    QueryPerformanceFrequency((LARGE_INTEGER *)&freq);

    do {
        QueryPerformanceCounter((LARGE_INTEGER *) &time2);
    } while((time2-time1) < us);
#else
    usleep(us);
#endif
}

bool ota_resp_is_ack = false;

/* read the response */
bool is_ack_resp_received( int comport )
{
  return ota_resp_is_ack;
}

/* read ack/message */
bool recv_ota(int comport)
{
    while (true)
    {
        memset(DATA_BUF, 0, ETX_OTA_PACKET_MAX_SIZE);
        uint16_t len = RS485_PollComport(comport, DATA_BUF, 4);
        if (len <= 0)
        {
            //printf("return, len %d\n", len);
            RS485_ConfigComport(comport, 500000);
            break;
        }

        ETX_OTA_RESP_* recv = (ETX_OTA_RESP_*)DATA_BUF;
        //printf("packet_type=%d, ", recv->packet_type);
        if (recv->packet_type == ETX_OTA_PACKET_TYPE_PRINT)
        {
            ETX_OTA_PRINT_* print = (ETX_OTA_PRINT_*)DATA_BUF;
            uint16_t data_len = RS485_PollComport(comport, &DATA_BUF[4], print->data_len);
            //printf("len %d: print->data_len %d ", data_len, print->data_len);

            DATA_BUF[4 + data_len] = '\0';
            printf("%s", &DATA_BUF[4]);
            //printf(".");
            //              printf("[%x]", print->data[0]);
                          //len -= (4 + data_len);
        }
        else if (recv->packet_type == ETX_OTA_PACKET_TYPE_RESPONSE)
        {
            ETX_OTA_RESP_* resp = (ETX_OTA_RESP_*)DATA_BUF;
            uint16_t resp_len = RS485_PollComport(comport, &DATA_BUF[4], sizeof(ETX_OTA_RESP_) - 4);
            //printf("len3 %d\n", resp_len);

            //TODO: Add CRC check
            ota_resp_is_ack = false;
            if (resp->status == ETX_OTA_ACK)
            {
                //ACK received
                ota_resp_is_ack = true;
            }
            //len -= (4 + resp_len);
        }

        RS485_ConfigComport(comport, 100);
        //len = RS485_PollComport(comport, DATA_BUF, 4);
        //printf("len4 %d\n", len);
    }

    return true;
}




/* Build the OTA START command */
int send_ota_start(int comport)
{
  uint16_t len;
  ETX_OTA_COMMAND_ *ota_start = (ETX_OTA_COMMAND_*)DATA_BUF;
  int ex = 0;

  memset(DATA_BUF, 0, ETX_OTA_PACKET_MAX_SIZE);

  ota_start->sof          = ETX_OTA_SOF;
  ota_start->packet_type  = ETX_OTA_PACKET_TYPE_CMD;
  ota_start->data_len     = 1;
  ota_start->cmd          = ETX_OTA_CMD_START;
  ota_start->crc          = 0x00;               //TODO: Add CRC
  ota_start->eof          = ETX_OTA_EOF;

  len = sizeof(ETX_OTA_COMMAND_);

  //send OTA START
  for(int i = 0; i < len; i++)
  {
    delay(1);
    if( RS485_SendByte(comport, DATA_BUF[i]) )
    {
      //some data missed.
      //printf("OTA START : Send Err\n");
      ex = -1;
      break;
    }
  }

  if( ex >= 0 )
  {
    recv_ota( comport );
    if( !is_ack_resp_received( comport ) )
    {
      //Received NACK
      //printf("OTA START : NACK\n");
      ex = -1;
    }
  }
  //printf("OTA START [ex = %d]\n", ex);
  return ex;
}

/* Build and Send the OTA END command */
uint16_t send_ota_end(int comport)
{
  uint16_t len;
  ETX_OTA_COMMAND_ *ota_end = (ETX_OTA_COMMAND_*)DATA_BUF;
  int ex = 0;

  memset(DATA_BUF, 0, ETX_OTA_PACKET_MAX_SIZE);

  ota_end->sof          = ETX_OTA_SOF;
  ota_end->packet_type  = ETX_OTA_PACKET_TYPE_CMD;
  ota_end->data_len     = 1;
  ota_end->cmd          = ETX_OTA_CMD_END;
  ota_end->crc          = 0x00;               //TODO: Add CRC
  ota_end->eof          = ETX_OTA_EOF;

  len = sizeof(ETX_OTA_COMMAND_);

  //send OTA END
  for(int i = 0; i < len; i++)
  {
    delay(1);
    if( RS485_SendByte(comport, DATA_BUF[i]) )
    {
      //some data missed.
      printf("OTA END : Send Err\n");
      ex = -1;
      break;
    }
  }

  if( ex >= 0 )
  {
    //printf("recv_ota 2\r\n");
    recv_ota( comport );
    if( !is_ack_resp_received( comport ) )
    {
      //Received NACK
      printf("OTA END : NACK\n");
      ex = -1;
    }
  }
  //printf("OTA END [ex = %d]\n", ex);
  return ex;
}

/* Build and send the OTA Header */
int send_ota_header(int comport, meta_info *ota_info)
{
  uint16_t len;
  ETX_OTA_HEADER_ *ota_header = (ETX_OTA_HEADER_*)DATA_BUF;
  int ex = 0;

  memset(DATA_BUF, 0, ETX_OTA_PACKET_MAX_SIZE);

  ota_header->sof          = ETX_OTA_SOF;
  ota_header->packet_type  = ETX_OTA_PACKET_TYPE_HEADER;
  ota_header->data_len     = sizeof(meta_info);
  ota_header->crc          = 0x00;               //TODO: Add CRC
  ota_header->eof          = ETX_OTA_EOF;

  memcpy(&ota_header->meta_data, ota_info, sizeof(meta_info) );

  len = sizeof(ETX_OTA_HEADER_);
  
  //send OTA Header
  for(int i = 0; i < len; i++)
  {
    delay(1);
    if( RS485_SendByte(comport, DATA_BUF[i]) )
    {
      //some data missed.
      printf("OTA HEADER : Send Err\n");
      ex = -1;
      break;
    }
  }

  if( ex >= 0 )
  {
    //printf("recv_ota 3\r\n");
    recv_ota( comport );
    if( !is_ack_resp_received( comport ) )
    {
      //Received NACK
      printf("OTA HEADER : NACK\n");
      ex = -1;
    }
  }
  //printf("OTA HEADER [ex = %d]\n", ex);
  return ex;
}

/* Build and send the OTA Data */
int send_ota_data(int comport, uint8_t *data, uint16_t data_len)
{
  uint16_t len;
  ETX_OTA_DATA_* ota_data = (ETX_OTA_DATA_*) DATA_BUF;
  int ex = 0;

  memset(DATA_BUF, 0, ETX_OTA_PACKET_MAX_SIZE);

  ota_data->sof          = ETX_OTA_SOF;
  ota_data->packet_type  = ETX_OTA_PACKET_TYPE_DATA;
  ota_data->data_len     = data_len;

  len = 4;

  //Copy the data
  memcpy(&DATA_BUF[len], data, data_len );
  len += data_len;
  uint32_t crc = 0u;        //TODO: Add CRC

  //Copy the crc
  memcpy(&DATA_BUF[len], (uint8_t*)&crc, sizeof(crc) );
  len += sizeof(crc);

  //Add the EOF
  DATA_BUF[len] = ETX_OTA_EOF;
  len++;

  //send OTA Data
  uint32_t sent = RS485_SendBuf(comport, DATA_BUF, len);
  if (sent != len)
  {
      //some data missed.
      printf("OTA DATA : Send Err, sent %d, len %d\n", sent, len);
      ex = -1;
  }

  if( ex >= 0 )
  {
    //printf("recv_ota 4\r\n");
    recv_ota( comport );
    if( !is_ack_resp_received( comport ) )
    {
      //Received NACK
      printf("OTA DATA : NACK\n");
      ex = -1;
    }
  }
  return ex;
}

int main(int argc, char *argv[])
{
  int comport;
  int bdrate = 1000000; //256000;       /* 115200 baud */
  char mode[]={'8','N','1',0}; /* *-bits, No parity, 1 stop bit */
  char bin_name[1024];
  int ex = 0;
  FILE *Fptr = NULL;

  do
  {
    if( argc <= 2 )
    {
      printf("Please feed the COM PORT number and the Application Image....!!!\n");
      printf("Example: .\\etx_ota_app.exe 8 ..\\..\\Application\\Debug\\Blinky.bin");
      ex = -1;
      break;
    }

    //get the COM port Number
    comport = atoi(argv[1]) -1;
    strcpy(bin_name, argv[2]);

    printf("Opening COM%d...\n", comport+1 );
    if (RS485_OpenComport(comport, bdrate, mode, 0) != 0)//INVALID_HANDLE_VALUE)
    {
      printf("Can not open comport\n");
      ex = -1;
      break;
    }

    int timeout = 1;
    while (timeout > 0)
    {
      ex = send_ota_start(comport);
      if (ex < 0/*fail*/)
         continue;

      timeout = 0;
      break;
    }
    if (timeout > 0)
    {
      printf("Device is not ready for programming, no connection\r\n\n");
      ex = -1;
      break;
    }


    printf("Opening Binary file : %s\n", bin_name);

    Fptr = fopen(bin_name,"rb");

    if( Fptr == NULL )
    {
      printf("Can not open %s\n", bin_name);
      ex = -1;
      break;
    }

    fseek(Fptr, 0L, SEEK_END);
    uint32_t file_size = ftell(Fptr);
    fseek(Fptr, 0L, SEEK_SET);

    //extract SREC 
    uint8_t line_buf[8192];
    int app_size;
    int bin_len = 0;
    int rec_num = 0;
    int line_len;
    int tot_len = 0;

    while (receive_line(Fptr, line_buf, &line_len) == 1)
    {
        tot_len += line_len;
        bin_len = extract_srec_line(line_buf, line_len, APP_BIN, bin_len);
        if (bin_len > 0)
        {
            app_size = bin_len;
        }
        rec_num++;
    }
    //printf("num records %d, num_s3 %d, app_size %d, tot_len %d, file_size %d\n", rec_num, num_s3, app_size, tot_len, file_size);

    //Send OTA Header
    meta_info ota_info;
    ota_info.package_size = app_size;
    ota_info.package_crc  = 0;          //TODO: Add CRC

    ex = send_ota_header( comport, &ota_info ); 
    if( ex < 0 )
    {
      printf("send_ota_header Err\n");
      break;
    }

    uint32_t bytes_sent = 0;
    do 
    {
      uint16_t size = 0;
      size = ((app_size - bytes_sent) > ETX_OTA_DATA_MAX_SIZE) ? ETX_OTA_DATA_MAX_SIZE :
                                        app_size - bytes_sent;

      ex = send_ota_data(comport, &APP_BIN[bytes_sent], size);
      if( ex < 0 )
      {
        printf("send_ota_data Err\n");
        break;
      }

      bytes_sent += size;
    } while (bytes_sent < (uint32_t)app_size);

    if( ex < 0 )
    {
      break;
    }

    //send OTA END command
    ex = send_ota_end(comport);
    if( ex < 0 )
    {
      printf("send_ota_end Err\n");
      break;
    }    

  } while (false);

  if(Fptr)
  {
      RS485_FlushComport(comport);
      RS485_CloseComport(comport);
//    fclose(Fptr);
  }

  if( ex < 0 )
  {
    printf("OTA ERROR\n");
  }
  return(ex);
}

