/*
 * slu.h
 *
 *  Created on: Apr 13, 2022
 *      Author: Margit Robert
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef INC_SLU_H_
#define INC_SLU_H_
/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/
#define SLU_OK          0
#define SLU_FAIL        1
#define SLU_ARG_ERROR     2

#define SLU_E8783A_ROW_TO_AUX     0x06
#define SLU_E8783A_ABUS1_TO_ROW   0x0E
#define SLU_E8783A_ABUS2_TO_ROW   0x16
#define SLU_E8783A_ABUS3_TO_ROW   0x1E
#define SLU_E8783A_ABUS4_TO_ROW   0x26

#define SLU_E8782A_UUT_TO_INST    0x06
#define SLU_E8782A_ROW_TO_AUX     0x09
#define SLU_E8782A_ABUS1_TO_INST  0x0E
#define SLU_E8782A_ABUS1_TO_ROW   0x11
#define SLU_E8782A_ABUS2_TO_INST  0x16
#define SLU_E8782A_ABUS2_TO_ROW   0x19
#define SLU_E8782A_ABUS3_TO_INST  0x1E
#define SLU_E8782A_ABUS3_TO_ROW   0x21
#define SLU_E8782A_ABUS4_TO_INST  0x26
#define SLU_E8782A_ABUS4_TO_ROW   0x29

/* Exported functions ------------------------------------------------------- */

void SluInit(SPI_HandleTypeDef *spi);
uint8_t SluReadReg(uint8_t address);
uint8_t SluWriteReg(uint8_t address, uint8_t data);
uint8_t SluSetRelay(uint8_t base, uint8_t relay);
char* SluGetModelNumber(void);

#endif /* INC_SLU_H_ */

/************************ (C) COPYRIGHT KonvolucioBt ***********END OF FILE****/
