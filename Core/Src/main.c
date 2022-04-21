/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include <stdio.h>
#include "LiveLed.h"
#include "hd44780.h"
#include "slu.h"
#include "mux.h"
#include "common.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

typedef enum _CtrlStatesTypeDef
{
  SDEV_START,                   //0
  SDEV_WAIT,                    //1
  SDEV_IDLE,                    //2
  SDEV_WAIT_FOR_CARD,           //3
  SDEV_NO_CARD,
  WORK,
  SDEV_WROK_U7178A,
  SDEV_ERROR,
  STOP,
  END,

}CtrlStatesTypeDef;

typedef struct _AppTypeDef
{
  uint8_t Address;
  uint8_t MuxRow;
  uint16_t FailCounter;
  uint8_t TestIndex;

  struct
  {
    CtrlStatesTypeDef Next;
    CtrlStatesTypeDef Curr;
    CtrlStatesTypeDef Pre;
  }State;

}DeviceTypeDef;


typedef enum _TestType
{
    TEST_ROW_OPEN = 0,
    TEST_ROW_CLOSE,
    TEST_AUX_OPEN,
    TEST_AUX_CLOSE

}TestType_t;

typedef struct
{
  char Name[25];
 TestType_t TestType;
  AnalogBus_t AnalogBus;
  uint8_t MaxRow;

} TestTableItem_t;


TestTableItem_t TestTable[] =
{
  /*01234567890123456789*/
  {"1. ABUS1-ROWs OPEN  ", TEST_ROW_OPEN, BUS_ABUS1, 64 },
  {"2. ABUS2-ROWs OPEN  ", TEST_ROW_OPEN, BUS_ABUS2, 64 },
  {"3. ABUS3-ROWs OPEN  ", TEST_ROW_OPEN, BUS_ABUS3, 64 },
  {"4. ABUS4-ROWs OPEN  ", TEST_ROW_OPEN, BUS_ABUS4, 64 },
  {"5. ABUS1-ROWs CLOSE ", TEST_ROW_CLOSE, BUS_ABUS1, 64 },
  {"6. ABUS2-ROWs CLOSE ", TEST_ROW_CLOSE, BUS_ABUS2, 64 },
  {"7. ABUS3-ROWs CLOSE ", TEST_ROW_CLOSE, BUS_ABUS3, 64 },
  {"8. ABUS4-ROWs CLOSE ", TEST_ROW_CLOSE, BUS_ABUS4, 64 },
  {"9. ABUS1-AUXs OPEN  ", TEST_AUX_OPEN, BUS_ABUS1, 64 },
  {"10. ABUS1-AUXs CLOSE", TEST_AUX_CLOSE, BUS_ABUS1, 64 },
};

#define TestTableCount (sizeof(TestTable)/sizeof(TestTableItem_t))

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
 SPI_HandleTypeDef hspi2;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */

DeviceTypeDef Device;
LiveLED_HnadleTypeDef hLiveLed;
char String[21];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI2_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */

void LiveLedOff(void);
void LiveLedOn(void);
void FailLedOn(void);
void FailLedOff(void);

void Backlight(uint8_t state);

uint8_t WorkTask(void);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

uint8_t WorkTask(void)
{
  static uint32_t timestamp = 0;
  switch(Device.State.Curr)
  {
    case SDEV_START:
    {
        Device.State.Next = SDEV_WAIT;
        timestamp = HAL_GetTick();
      break;
    }

    case SDEV_WAIT:
    {
      if((HAL_GetTick() - timestamp) > 1000 )
      {
        Device.State.Next = SDEV_WAIT_FOR_CARD;
      }
      break;
    }

    case SDEV_WAIT_FOR_CARD:
    {
      static uint8_t cardTryCnt;
      if(Device.State.Pre != Device.State.Curr)
      {
                        /*01234567890123456789*/
          LcdxyPuts(0,0," KARTYA AZONOSITASA ");
          timestamp = HAL_GetTick();
          cardTryCnt = 0;
      }
      if(HAL_GetTick() - timestamp > 1000)
      {
        uint8_t card = SluReadReg(SLU_REG_CARD_TYPE);
        if(SluGetModelName(String, card)== SLU_OK)
        {
            LcdxyPuts(0,1,String);
            if(card == 0x19)
              Device.State.Next = SDEV_WROK_U7178A;
            else
              Device.State.Next = WORK;
        }
        else
        {              /*01234567890123456789*/
          sprintf(String,"Keresem...Varj! %d ", cardTryCnt);
          LcdxyPuts(0,1,String);
          timestamp = HAL_GetTick();
          cardTryCnt++;
          if(cardTryCnt > 3)
            Device.State.Next = SDEV_NO_CARD;
        }
      }
      break;
    }

    case SDEV_NO_CARD:
    {
      if(Device.State.Pre != Device.State.Curr)
      {
                       /*01234567890123456789*/
          LcdxyPuts(0,1,"    Nincs kartya.   ");
          LcdxyPuts(0,3,"PIROS:Ismet");



      }
      break;
    }

    case SDEV_WROK_U7178A:
    {
      if(Device.State.Pre != Device.State.Curr)
      {
        LcdClrscr();
                     /*01234567890123456789*/
        LcdxyPuts(0,0,"      U7178A        ");

      }
      break;
    }
    case WORK:
    {
      if(Device.State.Pre != Device.State.Curr)
      {
        Device.TestIndex = 0;
        Device.MuxRow = 1;
        LcdxyPuts(0,0, TestTable[Device.TestIndex].Name);
        BusSetCurrent(TestTable[Device.TestIndex].AnalogBus);
      }

      if( Device.MuxRow < TestTable[Device.TestIndex].MaxRow)
      {

        if(TestTable[Device.TestIndex].TestType == TEST_ROW_OPEN)
        {
          MMuxSetRow(Device.MuxRow);

        }

        if(TestTable[Device.TestIndex].TestType == TEST_ROW_CLOSE)
        {
          if(TestTable[Device.TestIndex].AnalogBus == BUS_ABUS1)
            SluSetRelay(SLU_E8783A_ABUS1_TO_ROW, Device.MuxRow );

          if(TestTable[Device.TestIndex].AnalogBus == BUS_ABUS2)
            SluSetRelay(SLU_E8783A_ABUS2_TO_ROW, Device.MuxRow );

          if(TestTable[Device.TestIndex].AnalogBus == BUS_ABUS3)
            SluSetRelay(SLU_E8783A_ABUS3_TO_ROW, Device.MuxRow );

          if(TestTable[Device.TestIndex].AnalogBus == BUS_ABUS4)
            SluSetRelay(SLU_E8783A_ABUS4_TO_ROW, Device.MuxRow );

          MMuxSetRow(Device.MuxRow);

        }

        if(TestTable[Device.TestIndex].TestType == TEST_AUX_OPEN)
        {
          if(TestTable[Device.TestIndex].AnalogBus == BUS_ABUS1)
            SluSetRelay(SLU_E8783A_ABUS1_TO_ROW, Device.MuxRow );
          MMuxSetAux(Device.MuxRow);
        }

        if(TestTable[Device.TestIndex].TestType == TEST_AUX_CLOSE)
        {
          if(TestTable[Device.TestIndex].AnalogBus == BUS_ABUS1)
            SluSetRelay(SLU_E8783A_ABUS1_TO_ROW, Device.MuxRow );
          MMuxSetAux(Device.MuxRow);
        }


        memset(String,' ', sizeof(String));
        sprintf(String, "Row: %d", Device.MuxRow);
        LcdxyPuts(0,1, String);
        Device.MuxRow++;
        DelayMs(200);

      }
      else
      {
        Device.TestIndex++;
        Device.MuxRow = 1;
        if( Device.TestIndex > TestTableCount - 1 )
        {
          BusSetCurrent(BUS_OFF);
          Device.State.Next = END;
        }
        else
        {
          LcdxyPuts(0,0, TestTable[Device.TestIndex].Name);
          BusSetCurrent(TestTable[Device.TestIndex].AnalogBus);
        }
      }
      break;
    }
    case END:
    {               /*01234567890123456789*/
      LcdxyPuts(0,0, "     ELKESZULTEM    ");
    }

    case SDEV_ERROR:
    {
      break;
    }
  }

  Device.State.Pre = Device.State.Curr;
  Device.State.Curr = Device.State.Next;

  return DEVICE_OK;
}

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
  MX_SPI2_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */

  /*** Reset Everyting ***/
  HAL_GPIO_WritePin(RST_GPIO_Port, RST_Pin, GPIO_PIN_RESET);
  DelayMs(100);
  HAL_GPIO_WritePin(RST_GPIO_Port, RST_Pin, GPIO_PIN_SET);
  DelayMs(5);

  /*** LiveLed ***/
  hLiveLed.LedOffFnPtr = &LiveLedOff;
  hLiveLed.LedOnFnPtr = &LiveLedOn;
  hLiveLed.HalfPeriodTimeMs = 500;
  LiveLedInit(&hLiveLed);

  /*** LCD ***/
  LcdInit(LCD_FUNC_4B_2L, LCD_MODE_DISP, LCD_MODE_DISP);
         /*01234567890123456789*/
  LcdPuts("     Hello World    ");
  Backlight(1);
  SluInit(&hspi2);
  MuxInit(&hspi2);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  static uint32_t timestamp;


  while (1)
  {

    WorkTask();

   if(HAL_GetTick() - timestamp > 100)
   {
     timestamp = HAL_GetTick();
     //sprintf(string,"%lu", counter++);
     //LcdxyPuts(1,1, string);
   }

	LiveLedTask(&hLiveLed);
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
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
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief SPI2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI2_Init(void)
{

  /* USER CODE BEGIN SPI2_Init 0 */

  /* USER CODE END SPI2_Init 0 */

  /* USER CODE BEGIN SPI2_Init 1 */

  /* USER CODE END SPI2_Init 1 */
  /* SPI2 parameter configuration*/
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE END SPI2_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, ADC_CS_Pin|MMUX_CS_Pin|IMUX_CS_Pin|SLU_CS_Pin
                          |LCD_DB4_Pin|LCD_DB5_Pin|LCD_DB6_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, COM_EN_Pin|BUS4_EN_Pin|BUS3_EN_Pin|LCD_BKL_Pin
                          |TP14_Pin|TP15_Pin|SLU_RW_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, BUS2_EN_Pin|BUS1_EN_Pin|LIVE_LED_Pin|TP13_Pin
                          |SLU_STB_Pin|SLU_EN_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, RST_Pin|DAC_CS_Pin|LCD_E_Pin|LCD_RW_Pin
                          |LCD_RS_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(SLU_SLOT_GPIO_Port, SLU_SLOT_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LCD_DB7_GPIO_Port, LCD_DB7_Pin, GPIO_PIN_SET);

  /*Configure GPIO pins : ALevel_Pin BLevel_Pin */
  GPIO_InitStruct.Pin = ALevel_Pin|BLevel_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : ADC_CS_Pin COM_EN_Pin BUS4_EN_Pin BUS3_EN_Pin
                           LCD_BKL_Pin TP14_Pin TP15_Pin MMUX_CS_Pin
                           IMUX_CS_Pin SLU_CS_Pin SLU_RW_Pin LCD_DB4_Pin
                           LCD_DB5_Pin LCD_DB6_Pin */
  GPIO_InitStruct.Pin = ADC_CS_Pin|COM_EN_Pin|BUS4_EN_Pin|BUS3_EN_Pin
                          |LCD_BKL_Pin|TP14_Pin|TP15_Pin|MMUX_CS_Pin
                          |IMUX_CS_Pin|SLU_CS_Pin|SLU_RW_Pin|LCD_DB4_Pin
                          |LCD_DB5_Pin|LCD_DB6_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : BUS2_EN_Pin BUS1_EN_Pin LIVE_LED_Pin TP13_Pin
                           SLU_STB_Pin SLU_EN_Pin */
  GPIO_InitStruct.Pin = BUS2_EN_Pin|BUS1_EN_Pin|LIVE_LED_Pin|TP13_Pin
                          |SLU_STB_Pin|SLU_EN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : BTN3_Pin BTN2_Pin BTN1_Pin */
  GPIO_InitStruct.Pin = BTN3_Pin|BTN2_Pin|BTN1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : RST_Pin DAC_CS_Pin LCD_E_Pin LCD_RW_Pin
                           LCD_RS_Pin */
  GPIO_InitStruct.Pin = RST_Pin|DAC_CS_Pin|LCD_E_Pin|LCD_RW_Pin
                          |LCD_RS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : SLU_SLOT_Pin */
  GPIO_InitStruct.Pin = SLU_SLOT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(SLU_SLOT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LCD_DB7_Pin */
  GPIO_InitStruct.Pin = LCD_DB7_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LCD_DB7_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

/* printf -------------------------------------------------------------------*/
int _write(int file, char *ptr, int len)
{
  HAL_UART_Transmit(&huart1, (uint8_t*)ptr, len, 100);
  return len;
}

void ConsoleWrite(char *str)
{
  HAL_UART_Transmit(&huart1, (uint8_t*)str, strlen(str), 100);
}


void Backlight(uint8_t state)
{
  if(state)
    HAL_GPIO_WritePin(LCD_BKL_GPIO_Port, LCD_BKL_Pin, GPIO_PIN_SET);
  else
    HAL_GPIO_WritePin(LCD_BKL_GPIO_Port, LCD_BKL_Pin, GPIO_PIN_RESET);
}

/* LEDs ---------------------------------------------------------------------*/
void LiveLedOn(void)
{
  HAL_GPIO_WritePin(LIVE_LED_GPIO_Port, LIVE_LED_Pin, GPIO_PIN_SET);
}

void LiveLedOff(void)
{
  HAL_GPIO_WritePin(LIVE_LED_GPIO_Port, LIVE_LED_Pin, GPIO_PIN_RESET);
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
