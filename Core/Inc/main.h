/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

#define DEVICE_DEBUG_LEVEL    3

#if (DEVICE_DEBUG_LEVEL > 0)
#define  DeviceUsrLog(...)  {printf(__VA_ARGS__);\
                             printf("\r\n");}
#else
#define DeviceUsrLog(...)
#endif

#if (DEVICE_DEBUG_LEVEL > 1)

#define  DeviceErrLog(...)  {printf(VT100_ATTR_RED);\
                             printf("ERROR.DEVICE:") ;\
                             printf(__VA_ARGS__);\
                             printf(VT100_ATTR_RESET);\
                             printf("\r\n");}
#else
#define DeviceErrLog(...)
#endif

#if (DEVICE_DEBUG_LEVEL > 2)
#define  DeviceDbgLog(...)  {printf(VT100_ATTR_YELLOW);\
                             printf("DEBUG.DEVICE:") ;\
                             printf(__VA_ARGS__);\
                             printf(VT100_ATTR_RESET);\
                             printf("\r\n");}
#else
#define DeviceDbgLog(...)
#endif

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define ALevel_Pin GPIO_PIN_13
#define ALevel_GPIO_Port GPIOC
#define BLevel_Pin GPIO_PIN_14
#define BLevel_GPIO_Port GPIOC
#define ADC_CS_Pin GPIO_PIN_15
#define ADC_CS_GPIO_Port GPIOC
#define COM_EN_Pin GPIO_PIN_0
#define COM_EN_GPIO_Port GPIOC
#define BUS4_EN_Pin GPIO_PIN_1
#define BUS4_EN_GPIO_Port GPIOC
#define BUS3_EN_Pin GPIO_PIN_2
#define BUS3_EN_GPIO_Port GPIOC
#define LCD_BKL_Pin GPIO_PIN_3
#define LCD_BKL_GPIO_Port GPIOC
#define BUS2_EN_Pin GPIO_PIN_0
#define BUS2_EN_GPIO_Port GPIOA
#define BUS1_EN_Pin GPIO_PIN_1
#define BUS1_EN_GPIO_Port GPIOA
#define LIVE_LED_Pin GPIO_PIN_3
#define LIVE_LED_GPIO_Port GPIOA
#define BTN3_Pin GPIO_PIN_4
#define BTN3_GPIO_Port GPIOA
#define BTN2_Pin GPIO_PIN_5
#define BTN2_GPIO_Port GPIOA
#define BTN1_Pin GPIO_PIN_6
#define BTN1_GPIO_Port GPIOA
#define TP13_Pin GPIO_PIN_7
#define TP13_GPIO_Port GPIOA
#define TP14_Pin GPIO_PIN_4
#define TP14_GPIO_Port GPIOC
#define TP15_Pin GPIO_PIN_5
#define TP15_GPIO_Port GPIOC
#define RST_Pin GPIO_PIN_1
#define RST_GPIO_Port GPIOB
#define DAC_CS_Pin GPIO_PIN_12
#define DAC_CS_GPIO_Port GPIOB
#define MMUX_CS_Pin GPIO_PIN_6
#define MMUX_CS_GPIO_Port GPIOC
#define IMUX_CS_Pin GPIO_PIN_7
#define IMUX_CS_GPIO_Port GPIOC
#define SLU_CS_Pin GPIO_PIN_8
#define SLU_CS_GPIO_Port GPIOC
#define SLU_RW_Pin GPIO_PIN_9
#define SLU_RW_GPIO_Port GPIOC
#define SLU_SLOT_Pin GPIO_PIN_8
#define SLU_SLOT_GPIO_Port GPIOA
#define SLU_STB_Pin GPIO_PIN_11
#define SLU_STB_GPIO_Port GPIOA
#define SLU_EN_Pin GPIO_PIN_12
#define SLU_EN_GPIO_Port GPIOA
#define LCD_DB4_Pin GPIO_PIN_10
#define LCD_DB4_GPIO_Port GPIOC
#define LCD_DB5_Pin GPIO_PIN_11
#define LCD_DB5_GPIO_Port GPIOC
#define LCD_DB6_Pin GPIO_PIN_12
#define LCD_DB6_GPIO_Port GPIOC
#define LCD_DB7_Pin GPIO_PIN_2
#define LCD_DB7_GPIO_Port GPIOD
#define LCD_E_Pin GPIO_PIN_6
#define LCD_E_GPIO_Port GPIOB
#define LCD_RW_Pin GPIO_PIN_7
#define LCD_RW_GPIO_Port GPIOB
#define LCD_RS_Pin GPIO_PIN_8
#define LCD_RS_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
