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

typedef enum _AnalogBus
{
  BUS_OFF = 0,
  BUS_COMM = 1,
  BUS_ABUS1 = 2,
  BUS_ABUS2 = 3,
  BUS_ABUS3 = 4,
  BUS_ABUS4 = 5
}AnalogBus_t;




/* Exported functions ------------------------------------------------------- */
void MuxInit(SPI_HandleTypeDef *spi);
uint8_t MMuxSetRow(uint8_t select);
uint8_t MMuxSetAux(uint8_t select);

void BusSetCurrent(AnalogBus_t analog_bus);

#endif /* INC_MUX_H_ */

/************************ (C) COPYRIGHT KonvolucioBt ***********END OF FILE****/
