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

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

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
typedef struct{
uint8_t start_byte;
uint8_t flag_byte;
uint8_t stop_byte;

int16_t rocker_r_;
int16_t rocker_r1;
int16_t rocker_l_;
int16_t rocker_l1;
int16_t rocker_SWA;
int16_t rocker_SWB;
int16_t rocker_SWC;
int16_t rocker_SWD;
int16_t rocker_VRA;
int16_t rocker_VRB;

uint16_t error_protect;
uint16_t byte_lose;


}rc_;
rc_ rc_data;
uint8_t buf[255] = {0};
uint16_t CH[64];

uint8_t test = 0;

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
  MX_USART3_UART_Init();
  /* USER CODE BEGIN 2 */

  HAL_UARTEx_ReceiveToIdle_DMA(&huart3,buf,25);
  __HAL_DMA_DISABLE_IT(huart3.hdmarx,DMA_IT_HT);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while(1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    test ++;
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
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
  if(huart == &huart3)
  {
    if(buf[0] == 0x0F)//检验帧头
    {
      rc_data.start_byte = buf[0];

      rc_data.error_protect = (rc_data.flag_byte >> 3) & 0X01;
      rc_data.byte_lose = (rc_data.flag_byte >> 2) & 0X01;
      //获得丢帧保护位和传输错误位

      if(rc_data.error_protect | rc_data.byte_lose == 1)//位运算判断是否至少一位=1
      {
        return;//信号有问题时退出函数
      }

      uint8_t numPtr = 1;//当前解析到buf的哪一位
      uint8_t Ptr = 0;
      uint16_t temp = 0;

      for(int i = 0,p = 0;i < 10;i ++)
      {
        CH[i] = 0;
        for(int j = 0;j < 11;j ++,p ++)
        {

          if(p > 7)//p溢出时
          {
            p = 0;
            numPtr ++;
          }
          //取位->按照j填充到目标里->最后消掉11前的位数
          //处理一个全1的掩码
          temp = -1;
          //把第j位改成需要的内容  p代表当前在buf[numPtr]的从低到高第p位
          temp &= (1 & (buf[numPtr] >> p)) << j;
          CH[i] |= temp;

        }
        //处理后取后11位
        CH[i] &= 0x07FF;
      }

      //CH[0] = ((int16_t)buf[1] >> 0 | (int16_t)buf[2] << 8) & 0x07ff - 1024 + 784;

      rc_data.rocker_r_ = CH[0] - 1024;
      rc_data.rocker_r1 = CH[1] - 1024;
      rc_data.rocker_l_ = CH[2] - 1024;
      rc_data.rocker_l1 = CH[3] - 1024;
      rc_data.rocker_SWA = CH[4] - 1024;
      rc_data.rocker_SWB = CH[5] - 1024;
      rc_data.rocker_SWC = CH[6] - 1024;
      rc_data.rocker_SWD = CH[7] - 1024;
      rc_data.rocker_VRA = CH[8] - 1024;
      rc_data.rocker_VRB = CH[9] - 1024;

      rc_data.flag_byte = buf[23];

      


        HAL_UARTEx_ReceiveToIdle_DMA(&huart3,buf,25);
      __HAL_DMA_DISABLE_IT(huart3.hdmarx,DMA_IT_HT);
    }
  }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
  if(huart == &huart3)
  {
    HAL_UARTEx_ReceiveToIdle_DMA(&huart3,buf,25);
  __HAL_DMA_DISABLE_IT(huart3.hdmarx,DMA_IT_HT);
  }
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
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
