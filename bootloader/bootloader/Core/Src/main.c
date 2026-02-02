/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "etx_ota_update.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
//use the DEBUG_MODE for ST programmer via huart2 
/*#define DEBUG_MODE*/
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define MAJOR 0   // BL Major version Number
#define MINOR 2   // BL Minor version Number

#define APP_ADDRESS ETX_APP_FLASH_ADDR

#define BUFFER_SIZE (254)
static char buffer[BUFFER_SIZE] = { 0 };
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

//UART_HandleTypeDef huart2;
//UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
const uint8_t BL_Version[2] = { MAJOR, MINOR };
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
static void goto_application( void );
uint8_t uart_printf(const char *format, ...);

typedef void (*pFunction)(void);
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  #ifdef DEBUG_MODE
  MX_USART2_UART_Init();
  #endif//DEBUG_MODE
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  uart_printf("Starting Bootloader {%d.%d}\n", BL_Version[0], BL_Version[1]);

  //HAL_GPIO_WritePin(GPIOD, GPIO_PIN_8, GPIO_PIN_RESET);
  HAL_Delay (4000); /* 4sec delay*/
  /* USER CODE END 2 */

  /* Check the GPIO for 3 seconds */
  //uart_printf("Starting Firmware Download!!!\r\n");
#if 1
  /* OTA Request. Receive the data from the UART4 and flash */
  if( etx_ota_download_and_flash() != ETX_OTA_EX_OK )
  {
    /* Error. Don't process. */
    uart_printf("OTA Update : ERROR!!! HALT!!!\r\n");
    while( 1 );
  }
  else
  {
    /* Reset to load the new application */
    uart_printf("Firmware update is done!!! Rebooting...\r\n");
    etx_ota_send_print("\r\nFirmware update is done!!! Rebooting...\r\n");
//    HAL_NVIC_SystemReset();
  }
#endif

  // Jump to application
  goto_application();
  /* USER CODE END 2 */


  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1_BOOST);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV4;
  RCC_OscInitStruct.PLL.PLLN = 85;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}


/* USER CODE BEGIN 4 */
/* Jump to application */
static void goto_application(void)
{
    uint32_t jump_addr = *(uint32_t *)(APP_ADDRESS + 4);
    pFunction app = (pFunction)jump_addr;

    uart_printf ("Gonna jump to application\n");
    etx_ota_send_print("\r\nJump to application\r\n");

//    HAL_UART_DeInit(&huart2);
//    HAL_DeInit();

//    __disable_irq();
//    __set_MSP(*(uint32_t *)APP_ADDRESS);
    app();
}

uint8_t uart_printf(const char *format, ...) {
#ifdef DEBUG_MODE
	va_list args;
return 0;
	memset(buffer, 0, sizeof(buffer));
	va_start(args, format);
	vsprintf(buffer, format, args);
	perror(buffer);
	va_end(args);
	HAL_UART_Transmit(&huart2, (uint8_t*) buffer, strlen(buffer) < sizeof(buffer) - 1 ? (uint8_t) strlen(buffer) : (uint8_t) (sizeof(buffer) - 1),
	HAL_MAX_DELAY);
	return (uint8_t) strlen(buffer);
#else
  return 0;
#endif//DEBUG_MODE
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: uart_printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

