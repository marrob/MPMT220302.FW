/*
 * slu.c
 *
 *  Created on: Apr 13, 2022
 *      Author: Margit Robert
 */

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"
#include <stdio.h>
#include "slu.h"
#include "main.h"
#include "common.h"
#include "string.h"


/* Private define ------------------------------------------------------------*/
#define MCP23S08_IODIRA   0x00    /* IODIR – I/O DIRECTION REGISTER     */
#define MCP23S08_OLATA    0x0A    /* OUTPUT LATCH REGISTER              */
#define MCP23S08_GPIOA    0x09    /* GENERAL PURPOSE I/O PORT REGISTER  */

#define SPI_TIMEOUT_MS   100

#define SLU_MODEL_NUMBER_SIZE 20
#define SLU_CARD_TYPE_REG 0x01

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static SPI_HandleTypeDef *Spi;

typedef struct _SluCardTypeItem
{
  uint8_t Value;
  char ModelNumber[SLU_MODEL_NUMBER_SIZE];
}SluCardTypeItem_t;

SluCardTypeItem_t SluCardTypes[] =
{
    {0x01, "Agilent E6175A"},
    {0x02, "Agilent E6176A"},
    {0x03, "Agilent E6177A"},
    {0x18, "Agilent E7177A"},
    {0x19, "Agilent E7178A"},
    {0x20, "Agilent E7179A"},
    {0x04, "Agilent E6178B"},
    {0x05, "Agilent N9378A"},
    {0x06, "Agilent N9379A"},
    {0x07, "Agilent N9377A"},
    {0x0A, "Agilent E8792A"},
    {0x0B, "Agilent E8793A"},
    {0x43, "Agilent E8782A"},
    {0x47, "Agilent E8783A"},
    {0x14, "Agilent E8794A"},
    {0x32, "Agilent E6198B"},
};

/* Private function prototypes -----------------------------------------------*/
static inline void SluMcp23sWrite (uint8_t *data, uint8_t length);
static inline uint8_t SluMcp23sRead (uint8_t *data, uint8_t length);
/* Private user code ---------------------------------------------------------*/


void SluInit(SPI_HandleTypeDef *spi)
{

  Spi = spi;

  /***Power Supply Enable ***/
  HAL_GPIO_WritePin(SLU_EN_GPIO_Port, SLU_EN_Pin, GPIO_PIN_SET);



  /*** Select Card ***/
  HAL_GPIO_WritePin(SLU_SLOT_GPIO_Port, SLU_SLOT_Pin, GPIO_PIN_RESET);

}

/*
abus: SLU_E8783A_ABUS1..SLU_E8783A_ABUS4
relay:1..64
--- E8783A-AUX ---
Aux1:  Address:0x06 Data: 0x01 -> SluSetRelay(SLU_E8783A_ROW_TO_AUX, 1);
Aux8:  Address:0x06 Data: 0x80 -> SluSetRelay(SLU_E8783A_ROW_TO_AUX, 8);
Aux64: Address:0x0D Data: 0x80 -> SluSetRelay(SLU_E8783A_ROW_TO_AUX, 64);
--- E8783A-ABUS1 ---
Abus1-Row1:  Address:0x0E Data:0x01 -> SluSetRelay(SLU_E8783A_ABUS1_TO_ROW, 1);
Abus1-Row8:  Address:0x0E Data:0x80 -> SluSetRelay(SLU_E8783A_ABUS1_TO_ROW, 8);;
Abus1-Row64: Address:0x15 Data:0x80 -> SluSetRelay(SLU_E8783A_ABUS1_TO_ROW, 64);
--- E8783A-ABUS2 ---
Abus2-Row1:  Address:0x16 Data:0x01 -> SluSetRelay(SLU_E8783A_ABUS2_TO_ROW, 1)
Abus2-Row64: Address:0x1D Data:0x80 -> SluSetRelay(SLU_E8783A_ABUS2_TO_ROW, 64)
--- E8783A-ABUS3 ---
Abus3-Row1:  Address:0x16 Data:0x01 -> SluSetRelay(SLU_E8783A_ABUS3_TO_ROW, 1)
Abus3-Row64: Address:0x25 Data:0x80 -> SluSetRelay(SLU_E8783A_ABUS3_TO_ROW, 64)
--- E8783A-ABUS4 ---
Abus4-Row1:  Address:0x26 Data:0x01 -> SluSetRelay(SLU_E8783A_ABUS4_TO_ROW, 1)
Abus4-Row64: Address:0x2D Data:0x80 -> SluSetRelay(SLU_E8783A_ABUS4_TO_ROW, 64)

*/
uint8_t SluSetRelay(uint8_t base, uint8_t relay)
{
  if(relay == 0 || relay > 64)
    return SLU_ARG_ERROR;
  uint8_t address = 0;
  uint8_t data = 0;
  data = 1 << ((relay - 1) % 8);
  address = ((relay - 1) / 8) + base;
  printf("address:0x%02X data:0x%02X\r\n", address, data);
  SluWriteReg(address, data);

 return SLU_OK;
}

uint8_t SluReadReg(uint8_t address)
{
  uint8_t retval = 0x00;
  /*address-IO expander direction -> output*/
  SluMcp23sWrite((uint8_t[]){ 0x40, MCP23S08_IODIRA, 0x00}, 3);
  /*write address to output*/
  SluMcp23sWrite((uint8_t[]){ 0x40, MCP23S08_OLATA, address}, 3);
  /*data-IO expander direction -> input*/
  SluMcp23sWrite((uint8_t[]){ 0x42, MCP23S08_IODIRA, 0xFF}, 3);
  /*** Read From SLU ***/
  HAL_GPIO_WritePin(SLU_RW_GPIO_Port, SLU_RW_Pin, GPIO_PIN_SET);
  /*** Make a Strobe ***/
  HAL_GPIO_WritePin(SLU_STB_GPIO_Port, SLU_STB_Pin, GPIO_PIN_RESET);
  DelayMs(1);
  HAL_GPIO_WritePin(SLU_STB_GPIO_Port, SLU_STB_Pin, GPIO_PIN_SET);
  DelayMs(1);
  /*read data from expander*/
  retval = SluMcp23sRead((uint8_t[]){ 0x43, MCP23S08_IODIRA}, 2);
  /*** Read From SLU ***/
  HAL_GPIO_WritePin(SLU_RW_GPIO_Port, SLU_RW_Pin, GPIO_PIN_RESET);
  return retval;
}

uint8_t SluWriteReg(uint8_t address, uint8_t data)
{
  HAL_GPIO_WritePin(SLU_RW_GPIO_Port, SLU_RW_Pin, GPIO_PIN_RESET);
  /*address-IO expander direction -> output*/
  SluMcp23sWrite((uint8_t[]){ 0x40, MCP23S08_IODIRA, 0x00}, 3);
  /*write address to output*/
  SluMcp23sWrite((uint8_t[]){ 0x40, MCP23S08_OLATA, address}, 3);
  /*data-IO expander direction -> output*/
  SluMcp23sWrite((uint8_t[]){ 0x42, MCP23S08_IODIRA, 0x00}, 3);
  /*write data to data-io expander */
  SluMcp23sWrite((uint8_t[]){ 0x42, MCP23S08_OLATA, data}, 3);
  /*** Make a Strobe ***/
  HAL_GPIO_WritePin(SLU_STB_GPIO_Port, SLU_STB_Pin, GPIO_PIN_RESET);
  DelayMs(1);
  HAL_GPIO_WritePin(SLU_STB_GPIO_Port, SLU_STB_Pin, GPIO_PIN_SET);
  DelayMs(1);
  return SLU_OK;
}

char* SluGetModelNumber(void)
{
  static char model_number[SLU_MODEL_NUMBER_SIZE];
  strcpy(model_number, "Ismeretlen Kartya");

  uint8_t value = SluReadReg(SLU_CARD_TYPE_REG);

  for(uint8_t i=0; i < (sizeof(SluCardTypes)/sizeof(SluCardTypeItem_t)); i++)
  {
    if(SluCardTypes[i].Value == value)
      strcpy(model_number, SluCardTypes[i].ModelNumber);
  }
  return model_number;
}

static inline void SluMcp23sWrite (uint8_t *data, uint8_t length)
{
  HAL_GPIO_WritePin(SLU_CS_GPIO_Port, SLU_CS_Pin, GPIO_PIN_RESET);
  HAL_SPI_Transmit(Spi, data, length, SPI_TIMEOUT_MS);
  HAL_GPIO_WritePin(SLU_CS_GPIO_Port, SLU_CS_Pin, GPIO_PIN_SET);
}

static inline uint8_t SluMcp23sRead (uint8_t *data, uint8_t length)
{
  uint8_t retval[] = {0};
  HAL_GPIO_WritePin(SLU_CS_GPIO_Port, SLU_CS_Pin, GPIO_PIN_RESET);
  HAL_SPI_Transmit(Spi, data, length, SPI_TIMEOUT_MS);
  HAL_GPIO_WritePin(SLU_CS_GPIO_Port, SLU_CS_Pin, GPIO_PIN_SET);
  HAL_SPI_Receive(Spi, retval, 1, SPI_TIMEOUT_MS);
  return retval[0];
}



/************************ (C) COPYRIGHT KonvolucioBt ***********END OF FILE****/