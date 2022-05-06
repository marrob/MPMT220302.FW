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
#define SLU_MODEL_NUMBER_SIZE 20
#define SLU_RELAY_NAME_SIZE   10

typedef struct _SluRelayItem{
  uint8_t Value;
  char RelayName[SLU_RELAY_NAME_SIZE];
}SluRelayItem_t;


/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/
#define SLU_OK            0
#define SLU_FAIL          1
#define SLU_ARG_ERROR     2
#define SLU_UNKNWON       3

#define SLU_ROW_START             1
#define SLU_E8782A_INST_ROW_END   24

#define SLU_REG_CARD_TYPE               0x00
#define SLU_REG_CONFIG                  0x01
#define SLU_REG_STAT_CONT               0x02

#define SLU_BIT_STAT_MANUAL             0x40

/*U7178A 8-Ch. Heavy Duty */
#define SLU_REG_U7178A_CURRENT_SENSE    0x03
#define SLU_REG_U7178A_LOAD             0x04

/*E8783A E8782A*/
#define SLU_REG_E8783A_E8782A_BYPAS     0x04

#define SLU_REG_E8783A_ROW_TO_AUX       0x06
#define SLU_REG_E8783A_ABUS1_TO_ROW     0x0E
#define SLU_REG_E8783A_ABUS2_TO_ROW     0x16
#define SLU_REG_E8783A_ABUS3_TO_ROW     0x1E
#define SLU_REG_E8783A_ABUS4_TO_ROW     0x26

#define SLU_REG_E8782A_UUT_TO_INST      0x06
#define SLU_REG_E8782A_ROW_TO_AUX       0x09
#define SLU_REG_E8782A_ABUS1_TO_INST    0x0E
#define SLU_REG_E8782A_ABUS1_TO_ROW     0x11
#define SLU_REG_E8782A_ABUS2_TO_INST    0x16
#define SLU_REG_E8782A_ABUS2_TO_ROW     0x19
#define SLU_REG_E8782A_ABUS3_TO_INST    0x1E
#define SLU_REG_E8782A_ABUS3_TO_ROW     0x21
#define SLU_REG_E8782A_ABUS4_TO_INST    0x26
#define SLU_REG_E8782A_ABUS4_TO_ROW     0x29



/* Exported functions ------------------------------------------------------- */

void SluInit(SPI_HandleTypeDef *spi);
uint8_t SluReadReg(uint8_t address);
uint8_t SluWriteReg(uint8_t address, uint8_t data);
uint8_t SluSetRelay(uint8_t base, uint8_t relay);
uint8_t SluGetModelName(char *name, uint8_t value);
void SluCardSoftReset(void);
void SluOpenAllRelays(void);

//void SluBypassRelayOn(uint8_t pbx);
//void SluBypassRelayOff(uint8_t pbx);
extern SluRelayItem_t SLU_U7178A_Loads[];

#endif /* INC_SLU_H_ */

/************************ (C) COPYRIGHT KonvolucioBt ***********END OF FILE****/
