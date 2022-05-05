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
#define MEAS_HOLD_TIME_MS   50 //2022.05.05 - 5ms


typedef enum _CtrlStatesTypeDef
{
  SDEV_START,                   //0
  SDEV_WAIT,                    //1
  SDEV_IDLE,                    //2
  SDEV_WAIT_FOR_CARD,           //3
  SDEV_TST_MODE_SELECT,
  SDEV_WROK_U7178A,
  SDEV_BYPASS,
  SDEV_ROWS,
  SDEV_AUX,
  SDEV_INST_ROW,
  SDEV_INST_UUT,
  SDEV_FAIL_DETAIL,
  SDEV_NO_CARD,
  SDEV_NOT_SUPPERTED,
  SDEV_END,

}CtrlStatesTypeDef;

typedef enum _TestMode
{
  UUT_MODE_FULL,
  UUT_MODE_DEBUG
}UutMode_t;

typedef enum _TestType2
{
  TEST_TYPE_OPEN = 0,
  TEST_TYPE_CLOSE,
}TestType2_t;

typedef struct _AppTypeDef
{
  uint8_t Address;
  uint16_t FailCnt;
  uint16_t PassCnt;

  char UutName[20];
  char TestName[20];
  char ResultLine[20];

  UutMode_t UutMode;
  uint8_t FailAcceptFlag;
  CtrlStatesTypeDef SavedState;

  uint8_t RowsStart;
  uint8_t CurrentRow;
  uint8_t RowsEnd;

  TestType2_t CurrentTestType;

  /*
   * 1:ABUS1
   * 2:ABUS2
   * 3:ABUS3
   * 4:ABUS4
   * 5:UUTCOM
   */
  uint8_t CurrentAnalogBus;

  struct
  {
    CtrlStatesTypeDef Next;
    CtrlStatesTypeDef Curr;
    CtrlStatesTypeDef Pre;
  }State;

}DeviceTypeDef;


/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define E8782A_INST_ROW_END   24
#define E8782A_INST_ROW_START 1

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
char String[80];
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

uint8_t GetBtnGreen(void);
uint8_t GetBtnRed(void);
uint8_t GetBtnOrange(void);

void ConsoleWrite(char *str);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

uint8_t WorkTask(void)
{
  static uint32_t timestamp = 0;
  static uint8_t isBtnOrangeReleased = 0;
  static uint8_t isBtnGreenReleased = 0;
  switch(Device.State.Curr)
  {
    case SDEV_START:
    {
        Device.State.Next = SDEV_WAIT;
        timestamp = HAL_GetTick();
      break;
    }

    case SDEV_IDLE:
    {

      break;
    }

    case SDEV_WAIT:
    {
      if((HAL_GetTick() - timestamp) > 500 )
      {
        Device.State.Next = SDEV_WAIT_FOR_CARD;
      }
      break;
    }
    //01234567890123456789
    // KARTYA AZONOSITASA
    //
    //
    //
    case SDEV_WAIT_FOR_CARD:
    {
      static uint8_t cardTryCnt;
      if(Device.State.Pre != Device.State.Curr)
      {
          LcdClrscr();
          LcdxyPuts(0,0," KARTYA AZONOSITASA ");
          timestamp = HAL_GetTick();
          cardTryCnt = 0;
      }
      if(HAL_GetTick() - timestamp > 500)
      {
        uint8_t card = SluReadReg(SLU_REG_CARD_TYPE);
        Device.FailCnt = 0;
        Device.PassCnt = 0;
        MCP4812SetVolt(MCP48_CONF_A | MCP48_CONF_EN | MCP48_CONF_2x, 1.18);
        MCP4812SetVolt(MCP48_CONF_B | MCP48_CONF_EN | MCP48_CONF_2x, 2.23);

        if(SluGetModelName(String, card) == SLU_OK)
        {
          strcpy(Device.UutName, String);
          if(card == 0x19)
          {
            Device.State.Next = SDEV_WROK_U7178A;
          }
          else if(card == 0x47)
          {
            Device.RowsStart = 1;
            Device.RowsEnd = 64;
            Device.CurrentAnalogBus = BUS_ABUS1;
            Device.CurrentTestType = TEST_TYPE_OPEN;
            Device.State.Next = SDEV_TST_MODE_SELECT;
          }
          else if(card == 0x43)
          {
            Device.RowsStart = 1;
            Device.RowsEnd = 40;
            Device.CurrentAnalogBus = BUS_ABUS1;
            Device.CurrentTestType = TEST_TYPE_OPEN;
            Device.State.Next = SDEV_TST_MODE_SELECT;
            Device.State.Next = SDEV_BYPASS;
            //Device.State.Next =  SDEV_ROWS;
            //Device.State.Next = SDEV_AUX;
            //Device.State.Next = SDEV_INST_ROW;
          }
          else
            Device.State.Next = SDEV_NOT_SUPPERTED;

          Device.CurrentRow = Device.RowsStart;
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

    //01234567890123456789
    //      E8783A
    //    VALASZ MODOT!
    //
    //ZD>FULL    SG>DEBUG
    case SDEV_TST_MODE_SELECT:
    {
      if(Device.State.Pre != Device.State.Curr)
      {
        LcdClrscr();
        LcdxyPuts(7,0,Device.UutName);
        LcdxyPuts(0,1,"    VALASZ MODOT!   ");
        LcdxyPuts(0,3,"ZD>FULL     SG>DEBUG");
      }
      if(GetBtnGreen() && isBtnGreenReleased)
      {
        Device.State.Next = SDEV_BYPASS;
        //Device.State.Next =  SDEV_ROWS;
        //Device.State.Next = SDEV_AUX;
        Device.CurrentTestType = TEST_TYPE_OPEN;
        Device.UutMode = UUT_MODE_FULL;
        isBtnGreenReleased = 0;
      }
      if(GetBtnOrange() && isBtnOrangeReleased)
      {
        Device.State.Next = SDEV_BYPASS;
        Device.CurrentTestType = TEST_TYPE_OPEN;
        Device.UutMode = UUT_MODE_DEBUG;
        sprintf(String,"     %s            ", Device.UutName);
        isBtnOrangeReleased = 0;
      }
      break;
    }

    case SDEV_WROK_U7178A:
    {
      static uint8_t relay;

      if(Device.State.Pre != Device.State.Curr)
      {
        LcdClrscr();
                     /*01234567890123456789*/
        LcdxyPuts(0,0,"      U7178A        ");
        relay = 0;
        timestamp = HAL_GetTick();
      }

      if(HAL_GetTick() - timestamp > 1000)
      {
        timestamp = HAL_GetTick();
        if(relay > 7)
          relay = 0;
        SluWriteReg(SLU_REG_U7178A_LOAD, SLU_U7178A_Loads[relay].Value);
        LcdxyPuts(0,1, SLU_U7178A_Loads[relay].RelayName);
        relay++;
      }
      break;
    }

    case SDEV_FAIL_DETAIL:
    {
      //01234567890123456789
      //   HIBA RESZLETEI
      //01.DISC AB1 CLOSE
      //K901 R:200Ω OK
      //ZD>ISMET SG>KOVETKE
      static CtrlStatesTypeDef savedState;
      if(Device.State.Pre != Device.State.Curr)
      {
        savedState = Device.State.Pre;
        LcdClrscr();
        LcdxyPuts(0,0, "   HIBA RESZLETEI   ");
        LcdxyPuts(0,1, Device.TestName);
        LcdxyPuts(0,2, Device.ResultLine);
        LcdxyPuts(0,3, "ZD>ISMET SG>KOVETKE");
      }
      if(GetBtnGreen() && isBtnGreenReleased)
      {
        LcdClrscr();
        Device.State.Next = savedState;
        isBtnGreenReleased = 0;
      }

      if(GetBtnOrange() && isBtnOrangeReleased)
      {
        LcdClrscr();
        Device.FailAcceptFlag = 1;
        Device.State.Next = savedState;
        isBtnOrangeReleased = 0;
      }
      break;
    }

    //01234567890123456789
    //       E8783A
    //BYPASS CLOSE
    //K1 R<10ohm OK
    //OK:000 NOK:000
    case SDEV_BYPASS:
    {
      if(Device.State.Pre != Device.State.Curr)
      {
        LcdClrscr();
        LcdxyPuts(7,0,Device.UutName);
      }

      if(Device.CurrentTestType == TEST_TYPE_OPEN)
        sprintf(Device.TestName,"BYPS-ROW1-BUS%d OPN?", Device.CurrentAnalogBus);
      else if (Device.CurrentTestType == TEST_TYPE_CLOSE)
        sprintf(Device.TestName,"BYPS-ROW1-BUS%d CLS?",Device.CurrentAnalogBus);
      LcdxyPuts(0,1, Device.TestName);

      BusSetCurrent(Device.CurrentAnalogBus);
      MMuxSetRow(1);

      if(Device.CurrentTestType == TEST_TYPE_OPEN)
        SluWriteReg(SLU_REG_E8783A_E8782A_BYPAS, 0x00);


      if(strcmp(Device.UutName,"E8783A") == 0)
      {
        if(Device.CurrentAnalogBus == BUS_ABUS1)
        {
          SluSetRelay(SLU_REG_E8783A_ABUS1_TO_ROW, 1);
          if(Device.CurrentTestType == TEST_TYPE_CLOSE)
            SluWriteReg(SLU_REG_E8783A_E8782A_BYPAS, 0x01);
        }

        if(Device.CurrentAnalogBus ==BUS_ABUS2)
        {
          SluSetRelay(SLU_REG_E8783A_ABUS2_TO_ROW, 1);
          if(Device.CurrentTestType == TEST_TYPE_CLOSE)
            SluWriteReg(SLU_REG_E8783A_E8782A_BYPAS, 0x02);
        }

        if(Device.CurrentAnalogBus == BUS_ABUS3)
        {
          SluSetRelay(SLU_REG_E8783A_ABUS3_TO_ROW, 1);
          if(Device.CurrentTestType == TEST_TYPE_CLOSE)
            SluWriteReg(SLU_REG_E8783A_E8782A_BYPAS, 0x04);
        }
        if(Device.CurrentAnalogBus == BUS_ABUS4)
        {
          SluSetRelay(SLU_REG_E8783A_ABUS4_TO_ROW, 1);
          if(Device.CurrentTestType == TEST_TYPE_CLOSE)
            SluWriteReg(SLU_REG_E8783A_E8782A_BYPAS, 0x08);
        }
      }
      else if(strcmp(Device.UutName,"E8782A") == 0)
      {
        if(Device.CurrentAnalogBus == BUS_ABUS1)
        {
          SluSetRelay(SLU_REG_E8782A_ABUS1_TO_ROW, 1); //ABUS1 & ROW1: K125
          if(Device.CurrentTestType == TEST_TYPE_CLOSE)
            SluWriteReg(SLU_REG_E8783A_E8782A_BYPAS, 0x01);
        }

        if(Device.CurrentAnalogBus ==BUS_ABUS2)
        {
          SluSetRelay(SLU_REG_E8782A_ABUS2_TO_ROW, 1);
          if(Device.CurrentTestType == TEST_TYPE_CLOSE)
            SluWriteReg(SLU_REG_E8783A_E8782A_BYPAS, 0x02);
        }

        if(Device.CurrentAnalogBus == BUS_ABUS3)
        {
          SluSetRelay(SLU_REG_E8782A_ABUS3_TO_ROW, 1);
          if(Device.CurrentTestType == TEST_TYPE_CLOSE)
            SluWriteReg(SLU_REG_E8783A_E8782A_BYPAS, 0x04);
        }
        if(Device.CurrentAnalogBus == BUS_ABUS4)
        {
          SluSetRelay(SLU_REG_E8782A_ABUS4_TO_ROW, 1);
          if(Device.CurrentTestType == TEST_TYPE_CLOSE)
            SluWriteReg(SLU_REG_E8783A_E8782A_BYPAS, 0x08);
        }
      }

      DelayMs(MEAS_HOLD_TIME_MS);
      double volts = MCP3201GetVolt();
      double res = GetResistance(volts);
      uint8_t al = GetALevel();
      uint8_t bl = GetBLevel();
      uint8_t isPassed = 0;

      char resstr[8]; //R:200Ω
      if (res > 1000)
        sprintf(resstr, "R:>1K\xF4");
      else if(res < 10)
        sprintf(resstr, "R:<10\xF4");
      else
        sprintf(resstr, "R:%3.0f\xF4", res);

      if(Device.CurrentTestType == TEST_TYPE_OPEN)
      {
        if(al && bl)//0...25R
          isPassed = 0;
        else if ( !al && bl )//25R...225
          isPassed = 1;
        else if(!al && !bl)//225R... infinite
          isPassed = 0;
      }

      if(Device.CurrentTestType == TEST_TYPE_CLOSE)
      {
        if(al && bl)//0...25R
          isPassed = 1;
        else if ( !al && bl )//25R...225
          isPassed = 0;
        else if(!al && !bl)//225R... infinite
          isPassed = 0;
      }

      char rlystr[3];
      switch(Device.CurrentAnalogBus)
      {
        case BUS_ABUS1: strcpy(rlystr,"K1"); break;
        case BUS_ABUS2: strcpy(rlystr,"K2"); break;
        case BUS_ABUS3: strcpy(rlystr,"K3"); break;
        case BUS_ABUS4: strcpy(rlystr,"K4"); break;
        default: strcpy(rlystr,"?"); break;
      }

      //--- K101 R:200Ω OK ---
      if(isPassed)
        sprintf(Device.ResultLine, "%s %s OK ", rlystr, resstr);
      else
      {
        sprintf(Device.ResultLine, "%s %s NOK ", rlystr, resstr);
        if(Device.UutMode == UUT_MODE_DEBUG && !Device.FailAcceptFlag)
        {
          Device.SavedState = Device.State.Curr;
          Device.State.Next = SDEV_FAIL_DETAIL;
          break;
        }
      }

      if(isPassed)
        Device.PassCnt++;
      else
        Device.FailCnt++;

      Device.FailAcceptFlag = 0;
      LcdxyPuts(0, 2, Device.ResultLine);
      SluOpenAllRelays();


      sprintf(String, "OK:%d NOK:%d", Device.PassCnt, Device.FailCnt);
      LcdxyPuts(0,3, String);

      if(Device.CurrentAnalogBus == BUS_ABUS4)
      {
        Device.CurrentAnalogBus = BUS_ABUS1;
        if(Device.CurrentTestType == TEST_TYPE_CLOSE)
        {
          Device.State.Next = SDEV_ROWS;
          Device.CurrentRow = Device.RowsStart;
          Device.CurrentTestType = TEST_TYPE_OPEN;
        }
        else if(Device.CurrentTestType == TEST_TYPE_OPEN)
        {
          Device.CurrentTestType = TEST_TYPE_CLOSE;
          Device.CurrentAnalogBus = BUS_ABUS1;
        }
      }
      else
      {
        Device.CurrentAnalogBus++;
      }

      break;
    }
    case SDEV_ROWS:
    {
      if(Device.State.Pre != Device.State.Curr)
      {
        LcdClrscr();
        LcdxyPuts(7,0,Device.UutName);
      }

      if(Device.CurrentTestType == TEST_TYPE_OPEN)
        sprintf(Device.TestName, "ROW%02d-BUS%d OPN", Device.CurrentRow, Device.CurrentAnalogBus);
      else if (Device.CurrentTestType == TEST_TYPE_CLOSE)
        sprintf(Device.TestName, "ROW%02d-BUS%d CLS", Device.CurrentRow, Device.CurrentAnalogBus);
      LcdxyPuts(0,1, Device.TestName);

      if(Device.CurrentTestType == TEST_TYPE_OPEN)
      {
        BusSetCurrent(Device.CurrentAnalogBus);
        MMuxSetRow(Device.CurrentRow);
      }

      if(Device.CurrentTestType == TEST_TYPE_CLOSE)
      {
        BusSetCurrent(Device.CurrentAnalogBus);
        MMuxSetRow(Device.CurrentRow);
        if(strcmp(Device.UutName,"E8783A") == 0)
        {
          if(Device.CurrentAnalogBus == BUS_ABUS1)
            SluSetRelay(SLU_REG_E8783A_ABUS1_TO_ROW, Device.CurrentRow );

          if(Device.CurrentAnalogBus == BUS_ABUS2)
            SluSetRelay(SLU_REG_E8783A_ABUS2_TO_ROW, Device.CurrentRow );

          if(Device.CurrentAnalogBus == BUS_ABUS3)
            SluSetRelay(SLU_REG_E8783A_ABUS3_TO_ROW, Device.CurrentRow );

          if(Device.CurrentAnalogBus == BUS_ABUS4)
            SluSetRelay(SLU_REG_E8783A_ABUS4_TO_ROW, Device.CurrentRow );
        }
        else if(strcmp(Device.UutName,"E8782A") == 0)
        {
          if(Device.CurrentAnalogBus == BUS_ABUS1)
            SluSetRelay(SLU_REG_E8782A_ABUS1_TO_ROW, Device.CurrentRow );

          if(Device.CurrentAnalogBus == BUS_ABUS2)
            SluSetRelay(SLU_REG_E8782A_ABUS2_TO_ROW, Device.CurrentRow );

          if(Device.CurrentAnalogBus == BUS_ABUS3)
            SluSetRelay(SLU_REG_E8782A_ABUS3_TO_ROW, Device.CurrentRow );

          if(Device.CurrentAnalogBus == BUS_ABUS4)
            SluSetRelay(SLU_REG_E8782A_ABUS4_TO_ROW, Device.CurrentRow );
        }
      }

      DelayMs(MEAS_HOLD_TIME_MS);
      double volts = MCP3201GetVolt();
      double res = GetResistance(volts);
      uint8_t al = GetALevel();
      uint8_t bl = GetBLevel();
      uint8_t isPassed = 0;

      char resstr[8]; //R:200Ω
      if (res > 1000)
        sprintf(resstr, "R:>1K\xF4");
      else if(res < 10)
        sprintf(resstr, "R:<10\xF4");
      else
        sprintf(resstr, "R:%3.0f\xF4", res );

      if(Device.CurrentTestType == TEST_TYPE_CLOSE)
      {
        if(al && bl) //0...25R
          isPassed = 1;
        else if ( !al && bl ) //25R...225
          isPassed = 1;
        else if(!al && !bl)//225R... infinite
          isPassed = 0;
      }
      else if(Device.CurrentTestType == TEST_TYPE_OPEN)
      {
        if(al && bl) //0...25R
          isPassed = 0;
        else if (!al && bl)//25R...225
          isPassed = 0;
        else if(!al && !bl)//225R... infinite
          isPassed = 1;
      }

      if(strcmp(Device.UutName,"E8783A") == 0)
      {
        if(isPassed)
          sprintf(Device.ResultLine, "K%d%02d %s OK ", Device.CurrentAnalogBus, Device.CurrentRow, resstr);
        else
          sprintf(Device.ResultLine, "K%d%02d %s NOK ", Device.CurrentAnalogBus, Device.CurrentRow, resstr);
      }
      else if(strcmp(Device.UutName,"E8782A") == 0)
      {
        if(isPassed)
          sprintf(Device.ResultLine, "K%d%02d %s OK ", Device.CurrentAnalogBus, Device.CurrentRow + 24, resstr);
        else
          sprintf(Device.ResultLine, "K%d%02d %s NOK ", Device.CurrentAnalogBus, Device.CurrentRow + 24, resstr);
      }

      if(!isPassed)
      {
        if(Device.UutMode == UUT_MODE_DEBUG && !Device.FailAcceptFlag)
        {
          Device.State.Next = SDEV_FAIL_DETAIL;
          break;
        }
      }

      if(isPassed)
        Device.PassCnt++;
      else
        Device.FailCnt++;

      sprintf(String, "OK:%d NOK:%d", Device.PassCnt, Device.FailCnt);
      LcdxyPuts(0,3, String);

      LcdxyPuts(0, 2, Device.ResultLine);
      SluOpenAllRelays();

      if(Device.CurrentRow == Device.RowsEnd)
      {
        if(Device.CurrentAnalogBus == BUS_ABUS4 && Device.CurrentTestType == TEST_TYPE_CLOSE)
        {
          Device.State.Next = SDEV_AUX;
          //Device.State.Next = SDEV_END;
          Device.CurrentTestType = TEST_TYPE_OPEN;
          Device.CurrentAnalogBus = BUS_ABUS1;
        }
        else if(Device.CurrentAnalogBus == BUS_ABUS4 && Device.CurrentTestType == TEST_TYPE_OPEN)
        {
          Device.CurrentTestType = TEST_TYPE_CLOSE;
          Device.CurrentAnalogBus = BUS_ABUS1;
        }
        else
        {
          Device.CurrentAnalogBus++;
        }
        Device.CurrentRow = Device.RowsStart;
      }
      else
      {
        Device.CurrentRow++;
      }
      break;
    }

    case SDEV_AUX:
    {
      if(Device.State.Pre != Device.State.Curr)
      {
        LcdClrscr();
        LcdxyPuts(7,0,Device.UutName);
        BusSetCurrent(BUS_ABUS1);
      }

      if(Device.CurrentTestType == TEST_TYPE_OPEN)
        sprintf(Device.TestName, "AUX%02d-ROW%02d-BUS%d OPN",Device.CurrentRow, Device.CurrentRow, BUS_ABUS1);
      else if (Device.CurrentTestType == TEST_TYPE_CLOSE)
        sprintf(Device.TestName, "AUX%02d-ROW%02d-BUS%d CLS",Device.CurrentRow, Device.CurrentRow, BUS_ABUS1);
      LcdxyPuts(0,1, Device.TestName);

      if(strcmp(Device.UutName,"E8783A") == 0)
      {
        MMuxSetAux(Device.CurrentRow);
        SluSetRelay(SLU_REG_E8783A_ABUS1_TO_ROW, Device.CurrentRow );
        if(Device.CurrentTestType == TEST_TYPE_CLOSE)
        {
          SluSetRelay(SLU_REG_E8783A_ROW_TO_AUX, Device.CurrentRow);
        }
      }
      else if(strcmp(Device.UutName,"E8782A") == 0)
      {
        MMuxSetAux(Device.CurrentRow);
        SluSetRelay(SLU_REG_E8782A_ABUS1_TO_ROW, Device.CurrentRow );
        if(Device.CurrentTestType == TEST_TYPE_CLOSE)
        {
          SluSetRelay(SLU_REG_E8782A_ROW_TO_AUX, Device.CurrentRow);
        }
      }

      DelayMs(MEAS_HOLD_TIME_MS);
      double volts = MCP3201GetVolt();
      double res = GetResistance(volts);
      uint8_t al = GetALevel();
      uint8_t bl = GetBLevel();
      uint8_t isPassed = 0;

      char resstr[8]; //R:200Ω
      if (res > 1000)
        sprintf(resstr, "R:>1K\xF4");
      else if(res < 10)
        sprintf(resstr, "R:<10\xF4");
      else
        sprintf(resstr, "R:%3.0f\xF4", res );

      if(Device.CurrentTestType == TEST_TYPE_CLOSE)
      {
        if(al && bl)//0...25R
          isPassed = 1;
        else if ( !al && bl)//25R...225
          isPassed = 1;
        else if(!al && !bl)//225R... infinite
          isPassed = 0;
      }
      else if(Device.CurrentTestType == TEST_TYPE_OPEN)
      {
        if(al && bl)//0...25R
          isPassed = 0;
        else if (!al && bl)//25R...225
          isPassed = 0;
        else if(!al && !bl)//225R... infinite
          isPassed = 1;
      }

      //--- K901 NOK R:200Ω ---
      if(strcmp(Device.UutName,"E8783A") == 0)
      {
        if(isPassed)
          sprintf(Device.ResultLine, "K9%02d %s OK ", Device.CurrentRow, resstr);
        else
          sprintf(Device.ResultLine, "K9%02d %s NOK ", Device.CurrentRow, resstr);
      }
      else if(strcmp(Device.UutName,"E8782A") == 0)
      {
        if(isPassed)
          sprintf(Device.ResultLine, "K9%02d %s OK ", Device.CurrentRow + 24, resstr);
        else
          sprintf(Device.ResultLine, "K9%02d %s NOK ", Device.CurrentRow + 24, resstr);
      }

      if(!isPassed)
      {
        if(Device.UutMode == UUT_MODE_DEBUG && !Device.FailAcceptFlag)
        {
          Device.State.Next = SDEV_FAIL_DETAIL;
          break;
        }
      }

      LcdxyPuts(0, 2, Device.ResultLine);
      SluOpenAllRelays();

      if(isPassed)
        Device.PassCnt++;
      else
        Device.FailCnt++;

      sprintf(String, "OK:%d NOK:%d", Device.PassCnt, Device.FailCnt);
      LcdxyPuts(0,3, String);

      if(Device.CurrentRow == Device.RowsEnd)
      {
        if(Device.CurrentTestType == TEST_TYPE_CLOSE)
        {
          Device.CurrentTestType = TEST_TYPE_OPEN;
          Device.CurrentAnalogBus = BUS_ABUS1;

          if(strcmp(Device.UutName,"E8783A") == 0)
            Device.State.Next = SDEV_END;
          else if (strcmp(Device.UutName,"E8782A") == 0)
            Device.State.Next = SDEV_INST_ROW;
        }
        else if(Device.CurrentTestType == TEST_TYPE_OPEN)
        {
          Device.CurrentTestType = TEST_TYPE_CLOSE;
        }
        Device.CurrentRow = Device.RowsStart;
      }
      else
      {
        Device.CurrentRow++;
      }
      break;
    }

    case SDEV_INST_ROW:
    {
      if(Device.State.Pre != Device.State.Curr)
      {
        LcdClrscr();
        LcdxyPuts(7,0,Device.UutName);
      }

      if(Device.CurrentTestType == TEST_TYPE_OPEN)
        sprintf(Device.TestName, "INST%02d-ABUS%d OPN", Device.CurrentRow, Device.CurrentAnalogBus);
      else if (Device.CurrentTestType == TEST_TYPE_CLOSE)
        sprintf(Device.TestName, "INST%02d-ABUS%d CLS?", Device.CurrentRow, Device.CurrentAnalogBus);
      LcdxyPuts(0,1, Device.TestName);

      if(Device.CurrentTestType == TEST_TYPE_OPEN)
      {
        BusSetCurrent(Device.CurrentAnalogBus);
        IMuxSetRow(Device.CurrentRow);
      }

      if(Device.CurrentTestType == TEST_TYPE_CLOSE)
      {
        BusSetCurrent(Device.CurrentAnalogBus);
        MMuxSetRow(Device.CurrentRow);

        if(Device.CurrentAnalogBus == BUS_ABUS1)
          SluSetRelay(SLU_REG_E8782A_ABUS1_TO_INST, Device.CurrentRow );

        if(Device.CurrentAnalogBus == BUS_ABUS2)
          SluSetRelay(SLU_REG_E8782A_ABUS2_TO_INST, Device.CurrentRow );

        if(Device.CurrentAnalogBus == BUS_ABUS3)
          SluSetRelay(SLU_REG_E8782A_ABUS3_TO_INST, Device.CurrentRow );

        if(Device.CurrentAnalogBus == BUS_ABUS4)
          SluSetRelay(SLU_REG_E8782A_ABUS4_TO_INST, Device.CurrentRow );
      }

      DelayMs(MEAS_HOLD_TIME_MS);
      double volts = MCP3201GetVolt();
      double res = GetResistance(volts);
      uint8_t al = GetALevel();
      uint8_t bl = GetBLevel();
      uint8_t isPassed = 0;

      char resstr[8]; //R:200Ω
      if (res > 1000)
        sprintf(resstr, "R:>1K\xF4");
      else if(res < 10)
        sprintf(resstr, "R:<10\xF4");
      else
        sprintf(resstr, "R:%3.0f\xF4", res );

      if(Device.CurrentTestType == TEST_TYPE_CLOSE)
      {
        if(al && bl) //0...25R
          isPassed = 1;
        else if ( !al && bl ) //25R...225
          isPassed = 1;
        else if(!al && !bl)//225R... infinite
          isPassed = 0;
      }
      else if(Device.CurrentTestType == TEST_TYPE_OPEN)
      {
        if(al && bl) //0...25R
          isPassed = 0;
        else if (!al && bl)//25R...225
          isPassed = 0;
        else if(!al && !bl)//225R... infinite
          isPassed = 1;
      }

      if(isPassed)
        sprintf(Device.ResultLine, "K%d%02d %s OK ", Device.CurrentAnalogBus, Device.CurrentRow, resstr);
      else
        sprintf(Device.ResultLine, "K%d%02d %s NOK ", Device.CurrentAnalogBus, Device.CurrentRow, resstr);

      if(!isPassed)
      {
        if(Device.UutMode == UUT_MODE_DEBUG && !Device.FailAcceptFlag)
        {
          Device.State.Next = SDEV_FAIL_DETAIL;
          break;
        }
      }

      if(isPassed)
        Device.PassCnt++;
      else
        Device.FailCnt++;

      sprintf(String, "OK:%d NOK:%d", Device.PassCnt, Device.FailCnt);
      LcdxyPuts(0,3, String);

      LcdxyPuts(0, 2, Device.ResultLine);
      SluOpenAllRelays();

      if(Device.CurrentRow == E8782A_INST_ROW_END)
      {
        if(Device.CurrentAnalogBus == BUS_ABUS4 && Device.CurrentTestType == TEST_TYPE_CLOSE)
        {
          //Device.State.Next = SDEV_AUX;
          Device.State.Next = SDEV_END;
          Device.CurrentTestType = TEST_TYPE_OPEN;
          Device.CurrentAnalogBus = BUS_ABUS1;
        }
        else if(Device.CurrentAnalogBus == BUS_ABUS4 && Device.CurrentTestType == TEST_TYPE_OPEN)
        {
          Device.CurrentTestType = TEST_TYPE_CLOSE;
          Device.CurrentAnalogBus = BUS_ABUS1;
        }
        else
        {
          Device.CurrentAnalogBus++;
        }
        Device.CurrentRow = E8782A_INST_ROW_START;
      }
      else
      {
        Device.CurrentRow++;
      }
      break;
    }


    case SDEV_INST_UUT:
    {
      if(Device.State.Pre != Device.State.Curr)
      {
        LcdClrscr();
        LcdxyPuts(7,0,Device.UutName);
      }

      if(Device.CurrentTestType == TEST_TYPE_OPEN)
        sprintf(Device.TestName, "INST%02d-UUTComm OPN", Device.CurrentRow);
      else if (Device.CurrentTestType == TEST_TYPE_CLOSE)
        sprintf(Device.TestName, "INST%02d-UUTComm CLS?", Device.CurrentRow);
      LcdxyPuts(0,1, Device.TestName);

      BusSetCurrent(BUS_COMM);
      IMuxSetRow(Device.CurrentRow);

      if(Device.CurrentTestType == TEST_TYPE_CLOSE)
        SluSetRelay(SLU_REG_E8782A_UUT_TO_INST, Device.CurrentRow);


      DelayMs(MEAS_HOLD_TIME_MS);
      double volts = MCP3201GetVolt();
      double res = GetResistance(volts);
      uint8_t al = GetALevel();
      uint8_t bl = GetBLevel();
      uint8_t isPassed = 0;

      char resstr[8]; //R:200Ω
      if (res > 1000)
        sprintf(resstr, "R:>1K\xF4");
      else if(res < 10)
        sprintf(resstr, "R:<10\xF4");
      else
        sprintf(resstr, "R:%3.0f\xF4", res );

      if(Device.CurrentTestType == TEST_TYPE_CLOSE)
      {
        if(al && bl) //0...25R
          isPassed = 1;
        else if ( !al && bl ) //25R...225
          isPassed = 1;
        else if(!al && !bl)//225R... infinite
          isPassed = 0;
      }
      else if(Device.CurrentTestType == TEST_TYPE_OPEN)
      {
        if(al && bl) //0...25R
          isPassed = 0;
        else if (!al && bl)//25R...225
          isPassed = 0;
        else if(!al && !bl)//225R... infinite
          isPassed = 1;
      }

      if(isPassed)
        sprintf(Device.ResultLine, "K%d%02d %s OK ", Device.CurrentAnalogBus, Device.CurrentRow, resstr);
      else
        sprintf(Device.ResultLine, "K%d%02d %s NOK ", Device.CurrentAnalogBus, Device.CurrentRow, resstr);

      if(!isPassed)
      {
        if(Device.UutMode == UUT_MODE_DEBUG && !Device.FailAcceptFlag)
        {
          Device.State.Next = SDEV_FAIL_DETAIL;
          break;
        }
      }

      if(isPassed)
        Device.PassCnt++;
      else
        Device.FailCnt++;

      sprintf(String, "OK:%d NOK:%d", Device.PassCnt, Device.FailCnt);
      LcdxyPuts(0,3, String);

      LcdxyPuts(0, 2, Device.ResultLine);
      SluOpenAllRelays();

      if(Device.CurrentRow == E8782A_INST_ROW_END)
      {
        if(Device.CurrentAnalogBus == BUS_ABUS4 && Device.CurrentTestType == TEST_TYPE_CLOSE)
        {
          //Device.State.Next = SDEV_AUX;
          Device.State.Next = SDEV_END;
          Device.CurrentTestType = TEST_TYPE_OPEN;
          Device.CurrentAnalogBus = BUS_ABUS1;
        }
        else if(Device.CurrentAnalogBus == BUS_ABUS4 && Device.CurrentTestType == TEST_TYPE_OPEN)
        {
          Device.CurrentTestType = TEST_TYPE_CLOSE;
          Device.CurrentAnalogBus = BUS_ABUS1;
        }
        else
        {
          Device.CurrentAnalogBus++;
        }
        Device.CurrentRow = E8782A_INST_ROW_START;
      }
      else
      {
        Device.CurrentRow++;
      }
      break;
    }

    case SDEV_END:
    {
      //01234567890123456789
      //E8783A TESZT VEGE
      //  >>> FAILED <<<
      //OK:640 NOK:16
      //SG>UJRA"
      if(Device.State.Pre != Device.State.Curr)
      {
        LcdClrscr();
        sprintf(String,"%s TESZT VEGE", Device.UutName);
        LcdxyPuts(0,0, String);
        if(Device.FailCnt == 0)
          LcdxyPuts(0,1, "  >>> PASSSED <<<   ");
        else
          LcdxyPuts(0,1, "  >>> FAILED <<<    ");
        sprintf(String, "OK:%d NOK:%d", Device.PassCnt, Device.FailCnt);
        LcdxyPuts(0,2, String);
        LcdxyPuts(0,3, "SG>UJRA");
      }

      if(GetBtnOrange() && isBtnOrangeReleased)
      {
        LcdClrscr();
        Device.State.Next = SDEV_WAIT_FOR_CARD;
        isBtnOrangeReleased = 0;
      }
      break;
    }

    //01234567890123456789
    //    NINCS KARTYA
    //
    //
    //SG>UJRA"
    case SDEV_NO_CARD:
    {
      if(Device.State.Pre != Device.State.Curr)
      {
        LcdClrscr();
        LcdxyPuts(0,0,"    NINCS KARTYA    ");
        LcdxyPuts(0,3, "SG>UJRA");
      }
      if(GetBtnOrange())
      {
        Device.State.Next = SDEV_WAIT_FOR_CARD;
      }
      break;
    }

    //01234567890123456789
    //   NEM TAMOGATOTT
    //
    //
    //SG>UJRA"
    case SDEV_NOT_SUPPERTED:
    {
      if(Device.State.Pre != Device.State.Curr)
      {
        LcdClrscr();
        LcdxyPuts(0,0, "   NEM TAMOGATOTT   ");
        LcdxyPuts(0,3, "SG>UJRA");
      }

      if(GetBtnOrange())
      {
        LcdClrscr();
        Device.State.Next = SDEV_WAIT_FOR_CARD;
      }
      break;
    }
  }

  if(!GetBtnGreen())
    isBtnGreenReleased = 1;

  if(!GetBtnOrange())
    isBtnOrangeReleased = 1;

  Device.State.Pre = Device.State.Curr;
  Device.State.Curr = Device.State.Next;

  return DEVICE_OK;
}

/* USER CODE SDEV_END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE SDEV_END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE SDEV_END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE SDEV_END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_SPI2_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */

  //*** Reset Everyting ***
  HAL_GPIO_WritePin(RST_GPIO_Port, RST_Pin, GPIO_PIN_RESET);
  DelayMs(100);
  HAL_GPIO_WritePin(RST_GPIO_Port, RST_Pin, GPIO_PIN_SET);
  DelayMs(5);

  //*** LiveLed ***
  hLiveLed.LedOffFnPtr = &LiveLedOff;
  hLiveLed.LedOnFnPtr = &LiveLedOn;
  hLiveLed.HalfPeriodTimeMs = 500;
  LiveLedInit(&hLiveLed);

  //*** LCD ***
  LcdInit(LCD_FUNC_4B_2L, LCD_MODE_DISP, LCD_MODE_DISP);
               /*01234567890123456789*/
  LcdxyPuts(0, 0,"  PIN MATRIX TESTER ");
  sprintf(String, "%sV%s", DEVICE_NAME, DEVICE_PCB);
  LcdxyPuts(0, 1,String);//MPMT220302V00
  sprintf(String,"FW:%s", DEVICE_FW);
  LcdxyPuts(0, 2, String);//220428_1004
  sprintf(String,"%s", DEVICE_MNF);
  LcdxyPuts(0, 3, String);

  Backlight(1);
  SluInit(&hspi2);
  MuxInit(&hspi2);
  SluCardSoftReset();
  Device.UutMode = UUT_MODE_DEBUG;


 // SluSetRelay(SLU_REG_E8783A_ABUS1_TO_ROW, 1 );

 // SluSetRelay(SLU_REG_E8783A_ABUS1_TO_ROW, 64 );
  //0x14
  //0x15 0x80
  //0x1D
  //MMuxTest();


  /* USER CODE SDEV_END 2 */

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
    /* USER CODE SDEV_END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE SDEV_END 3 */
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

  /* USER CODE SDEV_END SPI2_Init 0 */

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
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE SDEV_END SPI2_Init 2 */

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

  /* USER CODE SDEV_END USART1_Init 1 */
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

  /* USER CODE SDEV_END USART1_Init 2 */

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

  /*Configure GPIO pins : RST_Pin LCD_E_Pin LCD_RW_Pin LCD_RS_Pin */
  GPIO_InitStruct.Pin = RST_Pin|LCD_E_Pin|LCD_RW_Pin|LCD_RS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : DAC_CS_Pin */
  GPIO_InitStruct.Pin = DAC_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(DAC_CS_GPIO_Port, &GPIO_InitStruct);

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
uint8_t red = 0;

/* Buttons--------------------------------------------------------------------*/
uint8_t GetBtnGreen(void)
{
  red = HAL_GPIO_ReadPin(BTN1_GPIO_Port, BTN1_Pin);
  return  red == GPIO_PIN_RESET;
}

uint8_t GetBtnRed(void)
{
  red = HAL_GPIO_ReadPin(BTN2_GPIO_Port, BTN2_Pin);
  return  red == GPIO_PIN_RESET;
}

uint8_t GetBtnOrange(void)
{
  red = HAL_GPIO_ReadPin(BTN3_GPIO_Port, BTN3_Pin);
  return  red == GPIO_PIN_RESET;
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
  /* USER CODE SDEV_END Error_Handler_Debug */
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
  /* USER CODE SDEV_END 6 */
}
#endif /* USE_FULL_ASSERT */
