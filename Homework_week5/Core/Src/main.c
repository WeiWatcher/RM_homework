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
#include "can.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "WEI_can.h"
#include <math.h>
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
DJI_Motor_Controller Line1;
CANInstance motor2;
DJI_Motor_Measure_s motor2_measure;

CAN_FilterTypeDef can_filter_conf;
uint8_t RTR = 0;
uint8_t IDE = 0;
uint8_t EXID = 0;

float t = 0;//计时变量
int16_t speed = 0;

uint8_t Data[8] = {0};

uint8_t can_rx_buff[8];
CAN_RxHeaderTypeDef rxconf; //接收头段

CAN_TxHeaderTypeDef Tempconf;

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
  MX_CAN1_Init();
  /* USER CODE BEGIN 2 */
  
  motor2.rx_id = 0x202;
  WEI_CAN_DJIContorller_Init(&Line1);
  WEI_CAN_MotorInstance_Init(&motor2_measure,&motor2,&Line1,2);

  //配置Line1的发送部分

  Line1.tx_id = 0x200;
  Line1.hcanHandle = &hcan1;
  Tempconf.RTR = 0;
  Tempconf.DLC = 8;
  Tempconf.IDE = 0;
  Tempconf.StdId = Line1.tx_id;

  Line1.txconf = &Tempconf;

  HAL_CAN_Start(&hcan1);
  //配置过滤器
  can_filter_conf.FilterMode = CAN_FILTERMODE_IDLIST;
  can_filter_conf.FilterScale = CAN_FILTERSCALE_16BIT;
  can_filter_conf.FilterFIFOAssignment = CAN_RX_FIFO0;
  can_filter_conf.SlaveStartFilterBank = 14;
  can_filter_conf.FilterIdLow = (0x202 << 5) | (RTR << 4) | (IDE << 3) | EXID;
  can_filter_conf.FilterBank = 1;
  can_filter_conf.FilterActivation = CAN_FILTER_ENABLE;

  HAL_CAN_ConfigFilter(&hcan1,&can_filter_conf);
  __HAL_CAN_ENABLE_IT(&hcan1,CAN_IT_RX_FIFO0_MSG_PENDING);



 



  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    speed = 1000 * sin(t);

    if(HAL_CAN_GetTxMailboxesFreeLevel(&hcan1) == 0);
    else
    {
      Data[0] = 0;
      Data[1] = 0;
      Data[2] = (uint8_t)(speed >> 8);
      Data[3] = (uint8_t)(speed & 0xFF);
      Data[4] = 0;
      Data[5] = 0;
      Data[6] = 0;
      Data[7] = 0;
    
      HAL_CAN_AddTxMessage(&hcan1,Line1.txconf,Data,&Line1.tx_mailbox);
    }
    t += 0.2f;

    HAL_Delay(10);


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
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef* hcan)
{
  test += 1;
  WEI_CAN_RxExchange(hcan,CAN_RX_FIFO0,&Line1);
}

void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef* hcan)
{
  test += 1;
  WEI_CAN_RxExchange(hcan,CAN_RX_FIFO1,&Line1);
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
