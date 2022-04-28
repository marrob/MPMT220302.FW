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
#define MCP23S08_IODIRA       0x00    /* IODIR – I/O DIRECTION REGISTER     */
#define MCP23S08_IOCONA       0x05    /* EXPANDER CONFIGURATION REGISTER    */
#define MCP23S08_OLATA        0x0A    /* OUTPUT LATCH REGISTER              */
#define MCP23S08_GPIOA        0x09    /* GENERAL PURPOSE I/O PORT REGISTER  */
#define MCP23S08_IOCON_HAEN   0x08    /* Hardware Address Enable bit        */

#define SPI_TIMEOUT_MS   100


/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static SPI_HandleTypeDef *Spi;

typedef struct _SluCardTypeItem
{
  uint8_t Value;
  char ModelName[SLU_MODEL_NUMBER_SIZE];
}SluCardTypeItem_t;





SluCardTypeItem_t SluCardTypes[] =
{
  {0x01, "E6175A"},
  {0x02, "E6176A"},
  {0x03, "E6177A"},
  {0x18, "E7177A"},
  {0x19, "E7178A"},
  {0x20, "E7179A"},
  {0x04, "E6178B"},
  {0x05, "N9378A"},
  {0x06, "N9379A"},
  {0x07, "N9377A"},
  {0x0A, "E8792A"},
  {0x0B, "E8793A"},
  {0x43, "E8782A"},
  {0x47, "E8783A"},
  {0x14, "E8794A"},
  {0x32, "E6198B"},
};

SluRelayItem_t SLU_U7178A_Loads[]=
{
  {0x01, "K21-CH1"},
  {0x02, "K22-CH2"},
  {0x04, "K23-CH3"},
  {0x08, "K24-CH4"},
  {0x10, "K25-CH5"},
  {0x20, "K26-CH6"},
  {0x40, "K27-CH7"},
  {0x80, "K28-CH8"},
};


/* Private function prototypes -----------------------------------------------*/
static inline uint8_t SluMcp23sRead (uint8_t mcpaddr, uint8_t reg);
static inline void SluMcp23sWrite (uint8_t mcpaddr, uint8_t reg, uint8_t value);
uint8_t Mcp23S08Read(uint8_t address);
/* Private user code ---------------------------------------------------------*/


void SluInit(SPI_HandleTypeDef *spi)
{

  Spi = spi;

  //***Power Supply Enable ***
  HAL_GPIO_WritePin(SLU_EN_GPIO_Port, SLU_EN_Pin, GPIO_PIN_SET);

  //*** Select Card ***
  HAL_GPIO_WritePin(SLU_SLOT_GPIO_Port, SLU_SLOT_Pin, GPIO_PIN_RESET);

  //*** Hardware Address of MCP23S08 Enable ***
  SluMcp23sWrite(0x00, MCP23S08_IOCONA, MCP23S08_IOCON_HAEN);
  SluMcp23sWrite(0x01, MCP23S08_IOCONA, MCP23S08_IOCON_HAEN);


 // uint8_t type = SluReadReg(0x00);
  //uint8_t type = Mcp23S08Read(0x55);

  //printf("%d", type);
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
  //printf("address:0x%02X data:0x%02X\r\n", address, data);
  SluWriteReg(address, data);

 return SLU_OK;
}


uint8_t SluReadReg(uint8_t address)
{
  uint8_t retval = 0x00;
  //address-IO expander direction -> output
  SluMcp23sWrite(0x00, MCP23S08_IODIRA, 0x00);
  //write address to output
  SluMcp23sWrite(0x00, MCP23S08_OLATA, address);
  //data-IO expander direction -> input
  SluMcp23sWrite(0x01, MCP23S08_IODIRA, 0xFF);
  //*** Read From SLU ***
  HAL_GPIO_WritePin(SLU_RW_GPIO_Port, SLU_RW_Pin, GPIO_PIN_SET);
  //*** Make Strobe ON ***
  HAL_GPIO_WritePin(SLU_STB_GPIO_Port, SLU_STB_Pin, GPIO_PIN_RESET);
  //read data from expander
  retval = SluMcp23sRead(0x01, MCP23S08_GPIOA);
  //*** Make Strobe OFF ***
  HAL_GPIO_WritePin(SLU_STB_GPIO_Port, SLU_STB_Pin, GPIO_PIN_SET);
  //*** Write To SLU ez legyen a default, hogy ne akdjon össze a busz***
  HAL_GPIO_WritePin(SLU_RW_GPIO_Port, SLU_RW_Pin, GPIO_PIN_RESET);
  return retval;
}

uint8_t SluWriteReg(uint8_t address, uint8_t data)
{
  HAL_GPIO_WritePin(SLU_RW_GPIO_Port, SLU_RW_Pin, GPIO_PIN_RESET);
  //address-IO expander direction -> output
  SluMcp23sWrite(0x00, MCP23S08_IODIRA, 0x00);
  //write address to output
  SluMcp23sWrite(0x00, MCP23S08_OLATA, address);
  //data-IO expander direction -> output
  SluMcp23sWrite(0x01, MCP23S08_IODIRA, 0x00);
  //write data to data-io expander
  SluMcp23sWrite(0x01, MCP23S08_OLATA, data);
  //*** Make a Strobe ***
  HAL_GPIO_WritePin(SLU_STB_GPIO_Port, SLU_STB_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(SLU_STB_GPIO_Port, SLU_STB_Pin, GPIO_PIN_SET);
  return SLU_OK;
}

void SluCardSoftReset(void)
{
  SluWriteReg(SLU_REG_STAT_CONT, 0x01);
}

void SluOpenAllRelays(void)
{
  SluWriteReg(SLU_REG_STAT_CONT, 0x20);
}
/*
 * pbx: PB1:1, PB2:2, PB3:3, B4:4
 */
//void SluBypassRelayOn(uint8_t pbx)
//{
//  SluWriteReg(SLU_REG_E8783A_E8782A_BYPAS, 1 << (pbx-1));
//}
//
//void SluBypassRelayOff(uint8_t pbx)
//{
//  SluWriteReg(SLU_REG_E8783A_E8782A_BYPAS, 1 << ((pbx-1) + 4));
//}

uint8_t SluGetModelName(char *name, uint8_t value)
{
  for(uint8_t i=0; i < (sizeof(SluCardTypes)/sizeof(SluCardTypeItem_t)); i++)
  {
    if(SluCardTypes[i].Value == value)
    {
      strcpy(name, SluCardTypes[i].ModelName);
      return SLU_OK;
    }
  }
  return SLU_UNKNWON;
}

uint8_t Mcp23S08Read(uint8_t address)
{
  uint8_t retval = 0x00;
  /*** Hardware Address of MCP23S08 Enable ***/
  SluMcp23sWrite(0x00, MCP23S08_IOCONA, MCP23S08_IOCON_HAEN);
  SluMcp23sWrite(0x01, MCP23S08_IOCONA, MCP23S08_IOCON_HAEN);
  /*address-IO expander direction -> output*/
  SluMcp23sWrite(0x00, MCP23S08_IODIRA, 0x00);
  /*write address to output*/
  SluMcp23sWrite(0x00, MCP23S08_OLATA, address);
  /*data-IO expander direction -> input*/
  SluMcp23sWrite(0x01, MCP23S08_IODIRA, 0xFF);
  /*read data from IO-expander*/
  retval = SluMcp23sRead(0x01, MCP23S08_GPIOA);
  return retval;
}


static inline void SluMcp23sWrite (uint8_t mcpaddr, uint8_t reg, uint8_t value)
{
  uint8_t tx[] = { 0x40 | mcpaddr << 1, reg, value};
  HAL_GPIO_WritePin(SLU_CS_GPIO_Port, SLU_CS_Pin, GPIO_PIN_RESET);
  HAL_SPI_Transmit(Spi, tx, sizeof(tx), SPI_TIMEOUT_MS);
  HAL_GPIO_WritePin(SLU_CS_GPIO_Port, SLU_CS_Pin, GPIO_PIN_SET);
}

static inline uint8_t SluMcp23sRead (uint8_t mcpaddr, uint8_t reg)
{
  uint8_t retval[] = {0};
  uint8_t tx[] = { 0x41 | mcpaddr << 1, reg};
  HAL_GPIO_WritePin(SLU_CS_GPIO_Port, SLU_CS_Pin, GPIO_PIN_RESET);
  HAL_SPI_Transmit(Spi, tx, sizeof(tx), SPI_TIMEOUT_MS);
  HAL_SPI_Receive(Spi, retval, 1, SPI_TIMEOUT_MS);
  HAL_GPIO_WritePin(SLU_CS_GPIO_Port, SLU_CS_Pin, GPIO_PIN_SET);
  return retval[0];
}

/************************ (C) COPYRIGHT KonvolucioBt ***********END OF FILE****/
