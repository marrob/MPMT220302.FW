/*
 * mux.c
 *
 *  Created on: Apr 14, 2022
 *      Author: Margit Robert
 */

/* Includes ------------------------------------------------------------------*/

#include "stm32f1xx_hal.h"
#include <stdio.h>
#include "mux.h"
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
/* Private function prototypes -----------------------------------------------*/
static inline void MMuxMcp23sWrite (uint8_t mcpaddr, uint8_t reg, uint8_t value);
static inline void IMuxMcp23sWrite(uint8_t mcpaddr, uint8_t reg, uint8_t value);

void MCP4812Set(uint8_t config, uint16_t value);
void MMuxExSxWrite(uint8_t e, uint8_t s);
void IMuxExSxWrite(uint8_t se);
uint16_t MCP3201Get(void);

/* Private user code ---------------------------------------------------------*/

void MuxInit(SPI_HandleTypeDef *spi)
{
  Spi = spi;

  //Hardware Address of MCP23S08 Enable
  MMuxMcp23sWrite(0x00, MCP23S08_IOCONA, MCP23S08_IOCON_HAEN);
  MMuxMcp23sWrite(0x01, MCP23S08_IOCONA, MCP23S08_IOCON_HAEN);

  //"Ex & Sx"-IO expander direction -> output
  MMuxMcp23sWrite(0x00, MCP23S08_IODIRA, 0x00);
  MMuxMcp23sWrite(0x01, MCP23S08_IODIRA, 0x00);

  //default value of "Ex & Sx"
  MMuxExSxWrite(0xFF, 0x00);

  /*"SxEx"-IO expander direction -> output*/
  IMuxMcp23sWrite(0x00, MCP23S08_IODIRA, 0x00);
  IMuxExSxWrite(0x30);
}

uint32_t ALevelHigh = 0;
uint32_t ALevelLow = 0;
uint32_t BLevelHigh = 0;
uint32_t BLevelLow = 0;
uint16_t adc = 0;
double Rr = 0;
double Vout = 0;
void MMuxTest(void)
{

  BusSetCurrent(BUS_ABUS1);
  MMuxSetRow(1);
  MCP4812SetVolt(MCP48_CONF_A | MCP48_CONF_EN | MCP48_CONF_2x, 1.18);
  MCP4812SetVolt(MCP48_CONF_B | MCP48_CONF_EN | MCP48_CONF_2x, 2.23);
  do
  {

//    adc = MCP3201Get();
    Vout = MCP3201GetVolt();

    Rr = GetResistance(Vout);

    if(GetALevel())
      ALevelHigh++;
    else
      ALevelLow++;

    if(GetBLevel())
      BLevelHigh++;
    else
      BLevelLow++;

    DelayMs(500);
  }while(1);
}


double GetResistance(double vout)
{
  double vdd=5.0, rh = 200, rm = 80, rl=200, rr=0;
  rr= (vout*rh + vout*rm + vout * rl - vdd * rm)/(vdd-vout);
  return rr;
}

void MCP4812SetVolt(uint8_t config, double volts)
{
  double lsb = 0.004;
  uint16_t dn = volts / lsb;
  MCP4812Set(config,dn);
}

void MCP4812Set(uint8_t config, uint16_t value)
{
  uint8_t data[] = {config | (value >> 6) , value << 2 };
  HAL_GPIO_WritePin(DAC_CS_GPIO_Port, DAC_CS_Pin, GPIO_PIN_RESET);
  HAL_SPI_Transmit(Spi, data, sizeof(data), SPI_TIMEOUT_MS);
  HAL_GPIO_WritePin(DAC_CS_GPIO_Port, DAC_CS_Pin, GPIO_PIN_SET);
}

/*
 * select: 1..64
 */
uint8_t MMuxSetRow(uint8_t select)
{
  uint8_t e=0;
  uint8_t s=0;

  switch(select)
  {
    case 1: e=0xFD; s=0x0F; break; //ROW1: E1:Y15 -> E:0xFD S:0x0F
    case 2: e=0xFD; s=0x0E; break; //ROW2: E1:Y14 -> E:0xFD S:0x0E
    case 3: e=0xFD; s=0x0D; break; //ROW2: E1:Y13 -> E:0xFD S:0x0D
    case 4: e=0xFD; s=0x0C; break;
    case 5: e=0xFD; s=0x0B; break;
    case 6: e=0xFD; s=0x0A; break;
    case 7: e=0xFD; s=0x09; break;
    case 8: e=0xFD; s=0x08; break;
    case 9: e=0xFD; s=0x07; break;
    case 10: e=0xFD; s=0x06; break;
    case 11: e=0xFD; s=0x05; break;
    case 12: e=0xFD; s=0x04; break;
    case 13: e=0xFD; s=0x03; break;
    case 14: e=0xFD; s=0x02; break;
    case 15: e=0xFD; s=0x01; break;
    case 16: e=0xFD; s=0x00; break; //ROW2: E1:Y0-> E:0xFD S:0x00

    case 17: e=0xFE; s=0x0F; break; //ROW17: E0:Y15 -> E:0xFE S:0x0F
    case 18: e=0xFE; s=0x0E; break;
    case 19: e=0xFE; s=0x0D; break;
    case 20: e=0xFE; s=0x0C; break;
    case 21: e=0xFE; s=0x0B; break;
    case 22: e=0xFE; s=0x0A; break;
    case 23: e=0xFE; s=0x09; break;
    case 24: e=0xFE; s=0x08; break;
    case 25: e=0xFE; s=0x07; break;
    case 26: e=0xFE; s=0x06; break;
    case 27: e=0xFE; s=0x05; break;
    case 28: e=0xFE; s=0x04; break;
    case 29: e=0xFE; s=0x03; break;
    case 30: e=0xFE; s=0x02; break;
    case 31: e=0xFE; s=0x01; break;
    case 32: e=0xFE; s=0x00; break; //ROW132: E0:Y0 -> E:0xFE S:0x00

    case 33: e=0xF7; s=0x0F; break; //ROW33: E3:Y15 -> E:0xF7 S:0x0F
    case 34: e=0xF7; s=0x0E; break;
    case 35: e=0xF7; s=0x0D; break;
    case 36: e=0xF7; s=0x0C; break;
    case 37: e=0xF7; s=0x0B; break;
    case 38: e=0xF7; s=0x0A; break;
    case 39: e=0xF7; s=0x09; break;
    case 40: e=0xF7; s=0x08; break;
    case 41: e=0xF7; s=0x07; break;
    case 42: e=0xF7; s=0x06; break;
    case 43: e=0xF7; s=0x05; break;
    case 44: e=0xF7; s=0x04; break;
    case 45: e=0xF7; s=0x03; break;
    case 46: e=0xF7; s=0x02; break;
    case 47: e=0xF7; s=0x01; break;
    case 48: e=0xF7; s=0x00; break; //ROW48: E3:Y0 -> E:0xF7 S:0x00

    case 49: e=0xFB; s=0x0F; break; //ROW49: E2:Y15 -> E:0xFB S:0x0F
    case 50: e=0xFB; s=0x0E; break;
    case 51: e=0xFB; s=0x0D; break;
    case 52: e=0xFB; s=0x0C; break;
    case 53: e=0xFB; s=0x0B; break;
    case 54: e=0xFB; s=0x0A; break;
    case 55: e=0xFB; s=0x09; break;
    case 56: e=0xFB; s=0x08; break;
    case 57: e=0xFB; s=0x07; break;
    case 58: e=0xFB; s=0x06; break;
    case 59: e=0xFB; s=0x05; break;
    case 60: e=0xFB; s=0x04; break;
    case 61: e=0xFB; s=0x03; break;
    case 62: e=0xFB; s=0x02; break;
    case 63: e=0xFB; s=0x01; break;
    case 64: e=0xFB; s=0x00; break; //ROW64: E2:Y0 -> E:0xFB S:0x00
  }

  if(select == 0)
    MMuxExSxWrite(0xFF, 0x00);
  else
    MMuxExSxWrite(e, s);

  return MUX_OK;
}

/*
 * select: 1..64
 */
uint8_t MMuxSetAux(uint8_t select)
{
  uint8_t e=0;
  uint8_t s=0;

  switch(select)
  {
    case 1: e=0xDF; s=0x0F; break; //AUX1: E5:Y15 -> E:0xDF S:0x0F
    case 2: e=0xDF; s=0x0E; break;
    case 3: e=0xDF; s=0x0D; break;
    case 4: e=0xDF; s=0x0C; break;
    case 5: e=0xDF; s=0x0B; break;
    case 6: e=0xDF; s=0x0A; break;
    case 7: e=0xDF; s=0x09; break;
    case 8: e=0xDF; s=0x08; break;
    case 9: e=0xDF; s=0x07; break;
    case 10: e=0xDF; s=0x06; break;
    case 11: e=0xDF; s=0x05; break;
    case 12: e=0xDF; s=0x04; break;
    case 13: e=0xDF; s=0x03; break;
    case 14: e=0xDF; s=0x02; break;
    case 15: e=0xDF; s=0x01; break;
    case 16: e=0xDF; s=0x00; break; //AUX16: E5:Y0 -> E:0xDF S:0x00

    case 17: e=0xEF; s=0x0F; break; //AUX17: E4:Y15 -> E:0xEF S:0x0F
    case 18: e=0xEF; s=0x0E; break;
    case 19: e=0xEF; s=0x0D; break;
    case 20: e=0xEF; s=0x0C; break;
    case 21: e=0xEF; s=0x0B; break;
    case 22: e=0xEF; s=0x0A; break;
    case 23: e=0xEF; s=0x09; break;
    case 24: e=0xEF; s=0x08; break;
    case 25: e=0xEF; s=0x07; break;
    case 26: e=0xEF; s=0x06; break;
    case 27: e=0xEF; s=0x05; break;
    case 28: e=0xEF; s=0x04; break;
    case 29: e=0xEF; s=0x03; break;
    case 30: e=0xEF; s=0x02; break;
    case 31: e=0xEF; s=0x01; break;
    case 32: e=0xEF; s=0x00; break; //AUX32: E4:Y0 -> E:0xEF S:0x00

    case 33: e=0x7F; s=0x0F; break; //AUX33: E7:Y15 -> E:0x7F S:0x0F
    case 34: e=0x7F; s=0x0E; break;
    case 35: e=0x7F; s=0x0D; break;
    case 36: e=0x7F; s=0x0C; break;
    case 37: e=0x7F; s=0x0B; break;
    case 38: e=0x7F; s=0x0A; break;
    case 39: e=0x7F; s=0x09; break;
    case 40: e=0x7F; s=0x08; break;
    case 41: e=0x7F; s=0x07; break;
    case 42: e=0x7F; s=0x06; break;
    case 43: e=0x7F; s=0x05; break;
    case 44: e=0x7F; s=0x04; break;
    case 45: e=0x7F; s=0x03; break;
    case 46: e=0x7F; s=0x02; break;
    case 47: e=0x7F; s=0x01; break;
    case 48: e=0x7F; s=0x00; break; //AUX48: E7:Y0 -> E:0x7F S:0x00

    case 49: e=0xBF; s=0x0F; break; //AUX49: E6:Y15 -> E:0xBF S:0x0F
    case 50: e=0xBF; s=0x0E; break;
    case 51: e=0xBF; s=0x0D; break;
    case 52: e=0xBF; s=0x0C; break;
    case 53: e=0xBF; s=0x0B; break;
    case 54: e=0xBF; s=0x0A; break;
    case 55: e=0xBF; s=0x09; break;
    case 56: e=0xBF; s=0x08; break;
    case 57: e=0xBF; s=0x07; break;
    case 58: e=0xBF; s=0x06; break;
    case 59: e=0xBF; s=0x05; break;
    case 60: e=0xBF; s=0x04; break;
    case 61: e=0xBF; s=0x03; break;
    case 62: e=0xBF; s=0x02; break;
    case 63: e=0xBF; s=0x01; break;
    case 64: e=0xBF; s=0x00; break; //AUX64: E6:Y0 -> E:0xBF S:0x00
  }

  if(select == 0)
    MMuxExSxWrite(0xFF, 0x00);
  else
    MMuxExSxWrite(e, s);

return MUX_OK;
}


void MMuxExSxWrite(uint8_t e, uint8_t s)
{
  /*"Ex"-IO expander direction -> output*/
  MMuxMcp23sWrite(0x00, MCP23S08_OLATA, e);
  /*"Sx"-IO expander direction -> output*/
  MMuxMcp23sWrite(0x01, MCP23S08_OLATA, s);
}



static inline void MMuxMcp23sWrite (uint8_t mcpaddr, uint8_t reg, uint8_t value)
{
  uint8_t tx[] = { 0x40 | mcpaddr << 1, reg, value};
  HAL_GPIO_WritePin(MMUX_CS_GPIO_Port, MMUX_CS_Pin, GPIO_PIN_RESET);
  HAL_SPI_Transmit(Spi, tx, sizeof(tx), SPI_TIMEOUT_MS);
  HAL_GPIO_WritePin(MMUX_CS_GPIO_Port, MMUX_CS_Pin, GPIO_PIN_SET);
}

/*
 * select: 1..24
 */
uint8_t IMuxSetRow(uint8_t select)
{
  uint8_t se = 0;

  switch(select)
  {
    case 1: se = 0x2F; break; //Inst1:  E0:Y15 -> 0x2F
    case 2: se = 0x2E; break; //Inst2:  E0:Y14 -> 0x2E
    case 3: se = 0x2D; break; //Inst3:  E0:Y13 -> 0x2D
    case 4: se = 0x2C; break; //Inst4:  E0:Y12 -> 0x2C
    case 5: se = 0x2B; break; //Inst5:  E0:Y11 -> 0x2B
    case 6: se = 0x2A; break; //Inst6:  E0:Y10 -> 0x2A
    case 7: se = 0x29; break; //Inst7:  E0:Y9  -> 0x29
    case 8: se = 0x28; break; //Inst8:  E0:Y8  -> 0x28
    case 9: se = 0x27; break; //Inst9:  E0:Y7  -> 0x27
    case 10: se = 0x26; break; //Inst10: E0:Y6  -> 0x26
    case 11: se = 0x25; break; //Inst11: E0:Y5  -> 0x25
    case 12: se = 0x24; break; //Inst12: E0:Y4  -> 0x24
    case 13: se = 0x23; break; //Inst13: E0:Y3  -> 0x23
    case 14: se = 0x22; break; //Inst14: E0:Y2  -> 0x22
    case 15: se = 0x21; break; //Inst15: E0:Y1  -> 0x21
    case 16: se = 0x20; break; //Inst16: E0:Y0  -> 0x20
    case 17: se = 0x1F; break; //Inst17: E1:Y15 -> 0x1F
    case 18: se = 0x1E; break; //Inst18: E1:Y14 -> 0x1E
    case 19: se = 0x1D; break; //Inst19: E1:Y13 -> 0x1D
    case 20: se = 0x1C; break; //Inst20: E1:Y12 -> 0x1C
    case 21: se = 0x1B; break; //Inst21: E1:Y11 -> 0x1B
    case 22: se = 0x1A; break; //Inst22: E1:Y10 -> 0x1A
    case 23: se = 0x19; break; //Inst23: E1:Y19 -> 0x19
    case 24: se = 0x18; break; //Inst24: E1:Y18 -> 0x18
  }

  if(select == 0)
    IMuxExSxWrite(0x30);
  else
    IMuxExSxWrite(se);

  return MUX_OK;
}

void IMuxExSxWrite(uint8_t se)
{
  IMuxMcp23sWrite(0x00, MCP23S08_OLATA, se);
}

static inline void IMuxMcp23sWrite(uint8_t mcpaddr, uint8_t reg, uint8_t value)
{
  uint8_t tx[] = { 0x40 | mcpaddr << 1, reg, value};
  HAL_GPIO_WritePin(IMUX_CS_GPIO_Port, IMUX_CS_Pin, GPIO_PIN_RESET);
  HAL_SPI_Transmit(Spi, tx, sizeof(tx), SPI_TIMEOUT_MS);
  HAL_GPIO_WritePin(IMUX_CS_GPIO_Port, IMUX_CS_Pin, GPIO_PIN_SET);
}

void BusSetCurrent(AnalogBus_t analog_bus)
{
  HAL_GPIO_WritePin(COM_EN_GPIO_Port, COM_EN_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(BUS4_EN_GPIO_Port, BUS4_EN_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(BUS3_EN_GPIO_Port, BUS3_EN_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(BUS2_EN_GPIO_Port, BUS2_EN_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(BUS1_EN_GPIO_Port, BUS1_EN_Pin, GPIO_PIN_RESET);

  switch (analog_bus)
  {
    case BUS_OFF: break;
    case BUS_COMM: HAL_GPIO_WritePin(COM_EN_GPIO_Port, COM_EN_Pin, GPIO_PIN_SET);break;
    case BUS_ABUS1: HAL_GPIO_WritePin(BUS1_EN_GPIO_Port, BUS1_EN_Pin, GPIO_PIN_SET);break;
    case BUS_ABUS2: HAL_GPIO_WritePin(BUS2_EN_GPIO_Port, BUS2_EN_Pin, GPIO_PIN_SET); break;
    case BUS_ABUS3: HAL_GPIO_WritePin(BUS3_EN_GPIO_Port, BUS3_EN_Pin, GPIO_PIN_SET);break;
    case BUS_ABUS4: HAL_GPIO_WritePin(BUS4_EN_GPIO_Port, BUS4_EN_Pin, GPIO_PIN_SET); break;
  }
}

uint8_t GetALevel(void)
{
  return HAL_GPIO_ReadPin(ALevel_GPIO_Port, ALevel_Pin);
}

uint8_t GetBLevel(void)
{
  return HAL_GPIO_ReadPin(BLevel_GPIO_Port, BLevel_Pin);
}

double MCP3201GetVolt(void)
{
   return MCP3201Get() * 0.001;
}
uint16_t MCP3201Get(void)
{
  uint16_t retval = 0;
  uint8_t data[] = {0x00, 0x00};
  HAL_GPIO_WritePin(ADC_CS_GPIO_Port, ADC_CS_Pin, GPIO_PIN_RESET);
  HAL_SPI_Receive(Spi, data, sizeof(data), SPI_TIMEOUT_MS);
  HAL_GPIO_WritePin(ADC_CS_GPIO_Port, ADC_CS_Pin, GPIO_PIN_SET);
  retval = data[0] << 8 | data [1];
  retval >>= 1;
  retval &= 0x1FFF;
  return retval;
}



/************************ (C) COPYRIGHT KonvolucioBt ***********END OF FILE****/
