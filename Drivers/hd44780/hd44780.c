/************************************************************
*   
*     HD44780U or Compatible Lcd Driver 
*           by
*         marrob Design 
*           ver:4.0
*       marrob50hz@gmail.com
***************************************************************/
#include "hd44780.h"

#define LCD_TYPE_4x20

/* Private define ------------------------------------------------------------*/
#if defined(LCD_TYPE_1x8)
  #define   LCD_FIRST_LINE_START  0x00
  #define   LCD_FIRST_LINE_END    0x07
#elif defined(LCD_TYPE_2x20)
  #define   LCD_FIRST_LINE_START  0x00
  #define   LCD_FIRST_LINE_END    0x13
  #define   LCD_SECOND_LINE_START 0x40
  #define   LCD_SECOND_LINE_END   0x53
#elif defined(LCD_TYPE_2x8)
  #define   LCD_FIRST_LINE_START  0x00
  #define   LCD_FIRST_LINE_END    0x07
  #define   LCD_SECOND_LINE_START 0x40
  #define   LCD_SECOND_LINE_END   0x47
#elif defined(LCD_TYPE_1x16)
  #define   LCD_FIRST_LINE_START  0x00
  #define   LCD_FIRST_LINE_END    0x0F
#elif defined(LCD_TYPE_2x16)
  #define   LCD_FIRST_LINE_START  0x00
  #define   LCD_FIRST_LINE_END    0x0F
  #define   LCD_SECOND_LINE_START 0x40
  #define   LCD_SECOND_LINE_END   0x4F
#elif defined(LCD_TYPE_4x16)
  #define   LCD_FIRST_LINE_START  0x00
  #define   LCD_FIRST_LINE_END    0x0F
  #define   LCD_SECOND_LINE_START 0x40
  #define   LCD_SECOND_LINE_END   0x4F
  #define   LCD_THIRD_LINE_START  0x00
  #define   LCD_THIRD_LINE_END    0x00
  #define   LCD_FOURTH_LINE_START 0x00
  #define   LCD_FORUTH_LINE_END   0x00
#elif defined(LCD_TYPE_4x20)
  #define   LCD_FIRST_LINE_START  0x00
  #define   LCD_FIRST_LINE_END    0x13
  #define   LCD_SECOND_LINE_START 0x40
  #define   LCD_SECOND_LINE_END   0x53
  #define   LCD_THIRD_LINE_START  0x14
  #define   LCD_THIRD_LINE_END    0x27
  #define   LCD_FOURTH_LINE_START 0x54
  #define   LCD_FORUTH_LINE_END   0x67
#endif

/* Private functions ---------------------------------------------------------*/

/*** Instruction Register ***/
void WriteInstrRegNoBusyCheck(uint8_t);
void WriteInstrRegNoBusyChk(uint8_t instr);
void WriteInstrRegBusyChk(uint8_t instr);
uint8_t ReadInstrReg(void);

/*** Data Register ***/
void WriteDataReg(uint8_t);
uint8_t ReadDataReg(void);

/*** Bus ***/
void BusWriteNibble(uint8_t data);
void BusWrite(uint8_t);
uint8_t BusRead(void);
uint8_t ReadAddressCount(void);

/*** Timing ***/
static void Delay(int32_t us);
static void DelayTPW(void);
static void DelayTH(void);

/* Private variables ---------------------------------------------------------*/
static uint8_t _LcdFunctionSet;


static void Delay(int32_t us)
{
  us = us * 72 / 4;
  while (us)
  {
    us--;
  }
}

static void DelayTPW(void)
{
  Delay(10);
}

static void DelayTH(void)
{
  Delay(10);
}

/*
 * sprintf(String,"Resistance:452\xF4);
 *
 */
void LcdxyPuts(uint8_t x, uint8_t y, const char *string)
{
  LcdxySetCursor(x,y);
  LcdPuts(string);
}

void LcdxySetCursor(uint8_t x,uint8_t y)
{
  #if defined(LCD_TYPE_1x8)
    Lcdddramaddress(y);
  #elif defined(LCD_TYPE_1x8)
    if(y==0){Lcdddramaddress(LCD_FIRST_LINE_START+x);}
    if(y==1){Lcdddramaddress(LCD_SECOND_LINE_START+x);}
  #elif defined(LCD_TYPE_1x16)  
    Lcdddramaddress(y);
  #elif defined(LCD_TYPE_2x16)
    if(y==0){Lcdddramaddress(LCD_FIRST_LINE_START+x);}
    if(y==1){Lcdddramaddress(LCD_SECOND_LINE_START+x);}
  #elif defined(LCD_TYPE_4x16)
    if(y==0){Lcdddramaddress(LCD_FIRST_LINE_START+x);}
    if(y==1){Lcdddramaddress(LCD_SECOND_LINE_START+x);}
    if(y==2){Lcdddramaddress(LCD_THIRD_LINE_START+x);}
    if(y==3){Lcdddramaddress(LCD_FOURTH_LINE_START+x);}
  #elif defined(LCD_TYPE_4x20)
    if(y==0){Lcdddramaddress(LCD_FIRST_LINE_START+x);}
    if(y==1){Lcdddramaddress(LCD_SECOND_LINE_START+x);}
    if(y==2){Lcdddramaddress(LCD_THIRD_LINE_START+x);}
    if(y==3){Lcdddramaddress(LCD_FOURTH_LINE_START+x);}
  #endif
}

void Lcdddramaddress(uint8_t DDRAMAddress)
{
  LcdWriteInstrReg((0x80|DDRAMAddress));
}

void LcdPuts(const char *string){
  do{
    if(*string=='\0')break;
      /*A szimbulomkat a sztringben 1..8 ig fordul elo, az Lcd pedig 0..7 ig v�rja. */ 
      if((0>=(*string)) || ((*string)<=8)) 
        LcdPutc((*string) - 1);
      else
        LcdPutc(*string);
    }while(*string++);
}

void LcdxyPutc(uint8_t x, uint8_t y, char c)
{
  LcdxySetCursor(x,y);
  LcdPutc(c);
}


void LcdClearLine(uint8_t line)
{
  LcdxySetCursor(0,line);
  for( uint8_t i=0; i<= LCD_FIRST_LINE_END - LCD_FIRST_LINE_START; i++)
  {
    LcdPutc(' ');
  }
}

void LcdClrscr(void)
{
  LcdWriteInstrReg(LCD_DISP_CLEAR);
  Delay(2000);
}

void LcdCursorHome(void)
{
  LcdWriteInstrReg(LCD_DISP_CURS_HOME);
  Delay(2000);
}

char LcdPutc(char c)
{
  WriteDataReg(c);
  return(c);
}

uint8_t LcdInit(uint8_t func,uint8_t entmode, uint8_t mode)
{
  Delay(15);
  WriteInstrRegNoBusyChk(LCD_FUNC_8B_1L);
  Delay(5);
  WriteInstrRegNoBusyChk(LCD_FUNC_8B_1L);
  Delay(1);
  WriteInstrRegNoBusyChk(LCD_FUNC_8B_1L);
  _LcdFunctionSet = func;

  WriteInstrRegNoBusyChk(func);
  LcdWriteInstrReg(func);

  LcdWriteInstrReg(LCD_ENTMOD_DEC | entmode);
  LcdWriteInstrReg(LCD_MODE_DISP_OFF | mode);
  LcdClrscr();
return(LCD_OK);
} 


void WriteInstrRegNoBusyChk(uint8_t instr)
{
  HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LCD_RW_GPIO_Port, LCD_RW_Pin, GPIO_PIN_RESET);
  BusWriteNibble(instr);
}

void LcdWriteInstrReg(uint8_t instr)
{
  HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LCD_RW_GPIO_Port, LCD_RW_Pin, GPIO_PIN_RESET);
  BusWrite(instr);
}

/**
  * Functional Name: LcdControl()
  * rerurn:
  * - None
  * Description:
  * Csak CU20029ECPB-W1J-d3
  */
void LcdControl(uint8_t value)
{
  LcdWriteInstrReg(_LcdFunctionSet);
  WriteDataReg(value);

}
/**
  * Saj�t szimb�lum defini�l�sa a CGRAM ba
  * Adress-> 1 es b�zius� (Egy�rtelm� legyen a stringben is az..)
  * 
  * Lcd egyedi karakterek bet�lt�se.
  * LcdWriteCGRAM(1,_SYM_GPS);
  * LcdWriteCGRAM(2,_SYM_PLUG);
  * LcdWriteCGRAM(3,_SYM_UP);
  * LcdWriteCGRAM(4,_SYM_DOWN);
  * LcdWriteCGRAM(5,_SYM_BIDIR);
  * Lcdputs("\1\2\3 Hello");
  */
void LcdWriteCGRAM(uint8_t address, const uint8_t *data)
{
  for(uint8_t i = 0; i<8; i++)
  {
    LcdWriteInstrReg(0x40 | i | (address-1)<<3 );
    WriteDataReg(data[i]);
  }
  LcdWriteInstrReg(LCD_DD_RAM_PTR);
}

void WriteDataReg(uint8_t data)
{
  HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(LCD_RW_GPIO_Port, LCD_RW_Pin, GPIO_PIN_RESET);
  BusWrite(data);
}

void BusWriteNibble(uint8_t byte)
{
  if(byte & 0x80)
    HAL_GPIO_WritePin(LCD_DB7_GPIO_Port, LCD_DB7_Pin, GPIO_PIN_SET);
  else
    HAL_GPIO_WritePin(LCD_DB7_GPIO_Port, LCD_DB7_Pin, GPIO_PIN_RESET);

  if(byte & 0x40)
    HAL_GPIO_WritePin(LCD_DB6_GPIO_Port, LCD_DB6_Pin, GPIO_PIN_SET);
  else
    HAL_GPIO_WritePin(LCD_DB6_GPIO_Port, LCD_DB6_Pin, GPIO_PIN_RESET);

  if(byte & 0x20)
    HAL_GPIO_WritePin(LCD_DB5_GPIO_Port, LCD_DB5_Pin, GPIO_PIN_SET);
  else
    HAL_GPIO_WritePin(LCD_DB5_GPIO_Port, LCD_DB5_Pin, GPIO_PIN_RESET);

  if(byte & 0x10)
    HAL_GPIO_WritePin(LCD_DB4_GPIO_Port, LCD_DB4_Pin, GPIO_PIN_SET);
  else
    HAL_GPIO_WritePin(LCD_DB4_GPIO_Port, LCD_DB4_Pin, GPIO_PIN_RESET);

  HAL_GPIO_WritePin(LCD_E_GPIO_Port, LCD_E_Pin, GPIO_PIN_SET);
  DelayTPW();
  HAL_GPIO_WritePin(LCD_E_GPIO_Port, LCD_E_Pin, GPIO_PIN_RESET);
  DelayTH();

  HAL_GPIO_WritePin(LCD_DB4_GPIO_Port, LCD_DB4_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(LCD_DB5_GPIO_Port, LCD_DB5_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(LCD_DB6_GPIO_Port, LCD_DB6_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(LCD_DB7_GPIO_Port, LCD_DB7_Pin, GPIO_PIN_SET);
}

void BusWrite(uint8_t byte)
{
  /*** MSN Write ***/
  if(byte & 0x80)
    HAL_GPIO_WritePin(LCD_DB7_GPIO_Port, LCD_DB7_Pin, GPIO_PIN_SET);
  else
    HAL_GPIO_WritePin(LCD_DB7_GPIO_Port, LCD_DB7_Pin, GPIO_PIN_RESET);

  if(byte & 0x40)
    HAL_GPIO_WritePin(LCD_DB6_GPIO_Port, LCD_DB6_Pin, GPIO_PIN_SET);
  else
    HAL_GPIO_WritePin(LCD_DB6_GPIO_Port, LCD_DB6_Pin, GPIO_PIN_RESET);

  if(byte & 0x20)
    HAL_GPIO_WritePin(LCD_DB5_GPIO_Port, LCD_DB5_Pin, GPIO_PIN_SET);
  else
    HAL_GPIO_WritePin(LCD_DB5_GPIO_Port, LCD_DB5_Pin, GPIO_PIN_RESET);

  if(byte & 0x10)
    HAL_GPIO_WritePin(LCD_DB4_GPIO_Port, LCD_DB4_Pin, GPIO_PIN_SET);
  else
    HAL_GPIO_WritePin(LCD_DB4_GPIO_Port, LCD_DB4_Pin, GPIO_PIN_RESET);


  HAL_GPIO_WritePin(LCD_E_GPIO_Port, LCD_E_Pin, GPIO_PIN_SET);
  DelayTPW();
  HAL_GPIO_WritePin(LCD_E_GPIO_Port, LCD_E_Pin, GPIO_PIN_RESET);
  DelayTH();

  /*** LSN Write ***/
  if(byte & 0x08)
    HAL_GPIO_WritePin(LCD_DB7_GPIO_Port, LCD_DB7_Pin, GPIO_PIN_SET);
  else
    HAL_GPIO_WritePin(LCD_DB7_GPIO_Port, LCD_DB7_Pin, GPIO_PIN_RESET);

  if(byte & 0x04)
    HAL_GPIO_WritePin(LCD_DB6_GPIO_Port, LCD_DB6_Pin, GPIO_PIN_SET);
  else
    HAL_GPIO_WritePin(LCD_DB6_GPIO_Port, LCD_DB6_Pin, GPIO_PIN_RESET);

  if(byte & 0x02)
    HAL_GPIO_WritePin(LCD_DB5_GPIO_Port, LCD_DB5_Pin, GPIO_PIN_SET);
  else
    HAL_GPIO_WritePin(LCD_DB5_GPIO_Port, LCD_DB5_Pin, GPIO_PIN_RESET);
  
  if(byte & 0x01)
    HAL_GPIO_WritePin(LCD_DB4_GPIO_Port, LCD_DB4_Pin, GPIO_PIN_SET);
  else
    HAL_GPIO_WritePin(LCD_DB4_GPIO_Port, LCD_DB4_Pin, GPIO_PIN_RESET);

  HAL_GPIO_WritePin(LCD_E_GPIO_Port, LCD_E_Pin, GPIO_PIN_SET);
  DelayTPW();
  HAL_GPIO_WritePin(LCD_E_GPIO_Port, LCD_E_Pin, GPIO_PIN_RESET);
  DelayTH();

  /*** Default ***/
  HAL_GPIO_WritePin(LCD_DB4_GPIO_Port, LCD_DB4_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(LCD_DB5_GPIO_Port, LCD_DB5_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(LCD_DB6_GPIO_Port, LCD_DB6_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(LCD_DB7_GPIO_Port, LCD_DB7_Pin, GPIO_PIN_SET);

}

