/*
 * mux.h
 *
 *  Created on: Apr 14, 2022
 *      Author: Margit Robert
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef INC_MUX_H_
#define INC_MUX_H_
/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/
#define MUX_OK          0
#define MUX_FAIL        1
#define MUX_ARG_ERROR   2


#define MCP48_CONF_B          1<<7
#define MCP48_CONF_A          0<<7
#define MCP48_CONF_1x         1<<5
#define MCP48_CONF_2x         0<<5
#define MCP48_CONF_SH         0<<4
#define MCP48_CONF_EN         1<<4

typedef enum _AnalogBus
{
  BUS_OFF = 0,
  BUS_ABUS1 = 1,
  BUS_ABUS2 = 2,
  BUS_ABUS3 = 3,
  BUS_ABUS4 = 4,
  BUS_COMM = 5

}AnalogBus_t;




/* Exported functions ------------------------------------------------------- */
void MuxInit(SPI_HandleTypeDef *spi);
uint8_t MMuxSetRow(uint8_t select);
uint8_t MMuxSetAux(uint8_t select);
uint8_t IMuxSetRow(uint8_t select);
void MMuxTest(void);
void BusSetCurrent(AnalogBus_t analog_bus);
double MCP3201GetVolt(void);
double GetResistance(double vout);
uint8_t GetBLevel(void);
uint8_t GetALevel(void);

void MCP4812SetVolt(uint8_t config, double volts);


#endif /* INC_MUX_H_ */

/************************ (C) COPYRIGHT KonvolucioBt ***********END OF FILE****/
