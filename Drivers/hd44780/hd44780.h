/******************** (C) COPYRIGHT 2012 marrob Design *************************
* File Name          : HD44780.h
* Author             : Margit R�bert
* Date First Issued  : 28/06/2007
* Description        : HD44780 driver
********************************************************************************/
/***************************************************************
2007.11.11:  Comment: 
    A novatek NT7603 driver a datasheet-el szemben m�gsem 
    t�mogatja a busy fleg figyel�ses kommunik�ci�t. ...... ver2
***************************************************************/
/***************************************************************
2008.06.06: update for:Atmega16 
***************************************************************/
/***************************************************************
2008.9.25:    update for:Atmeg32 and Free RTOS .......ver3
***************************************************************/
/***************************************************************
2008.9.25:    Comment:
    * Az LCD parancs fuggvenyei helyett lehetne macrokat 
      hasznalni, igy takarekoskondin lehetne a Stackkel
    * Megvalositando a 4x20-os LCD kezeles
    *   Meg a 8 bites vezerles 
    *   ha ezek megvalosulnak akkor veglegesnek tekintheto
***************************************************************/
/***************************************************************
2009.02.03  -hozza masoltam a 4 Soros LCD kezelest de meg nem 
    portolttam hozza, Csabinal mar mukodott
***************************************************************/
/***************************************************************
2009.02.10  -hozza masoltam a 4 Soros LCD kezelest veglegesitve
    +makro kiegeszitesek es javitasok....
    versio szam ver3.2
***************************************************************/
/***************************************************************
2009.02.23:
    -MCP C18-ra is megy a forditas
    -Delayt NEM TUD azt a kovetkezo javitasban ...
        versio szam ver3.3
***************************************************************/
/***************************************************************
2009.07.03: 
    -Atmega128 Tamogatasa
    -Az LCD a uC barmely portjara, barmilyen sorrendben elhelyzheto
    -A Port definiciok ujar irasa, nem kopatibilis a 3.x verziokkal
    -A parancsok kompatibilisek a 3.x verziokkal
    -Uj parancs: xyLCDputs(x,y,*stirng); + ehhez kell a helyes "#define LCD_TYPE_XxX" definico
    -A port definiciok GNUC-vel kopatibilis 100%-ra
    -A kod lerovidult es egyszerusodott
    -Az LCDinit() parameterezesenek hibait javitottam 
    -UJ VERSIO SZAM: ver:4.0
***************************************************************/
/***************************************************************
2011.06.03: 
  Egyedi karakterek k�sz�t�s�nek lehetos�ge...
LCDWriteCGRAM
    -UJ VERSIO SZAM: ver:4.1
***************************************************************/
/****************************************************
2011.07.13
- Uj: LCDClearLine(x)
ver:4.2
*****************************************************/
/****************************************************
*
*     Supported LCD Type  
* 
*****************************************************
LCD_TYPE_1x8
LCD_TYPE_2x8
LCD_TYPE_1x16
LCD_TYPE_2x16
LCD_TYPE_4x16
*****************************************************/

/****************************************************
*
*     LCD Commands
*   
*****************************************************
LCDLightON();           //LCD Hattervilagitas Be
LCDLightOFF();            //LCD Hattervilagitas Ki
LCDclrscr();            //LCD Kepernyo torlese
LCDoff();             //LCD Kikapcsolasa
LCDddramaddress(unsigned char);   //LCD Karater Mutato Irasa  (nem ajanlatos kivurol hasznalni)
LCDCommand(unsigned char);      //LCD vezro parancs kuldese (nem ajanlatos kivurol hasznalni)
LCDCursorHome();          //LCD Kocsi Vissza                  
LCDShiftRight();          //LCD Tartalom Shiftelese Jobbra
LCDShiftLeft();           //LCD Tartalom Shiftelese Balra
char LCDInit(func,entmode,mode);  //LCD Inicializacio (Lasd mintapelda)
char LCDputc(c);          //Karakter irasa
void LCDputs(*string);        //Karakter lanc irasa
void xyLCDputs(x,y,*string);    //Pozicionalt karakter lanc irasa (Helyesen kofniguralt #define LCD_TYPE_XxX-ra 
van szukseg, csak 2 soros LCD-re parameterzett LCD-n hasznalhato ( LCDinit(xxxxx_2L,xxxxxx,xxxxxx); )
*****************************************************/
/****************************************************
2011.12.26
Frisstitve LFO GPS II
****************************************************/
/***************************************************************
2012.01.17:
- Uj verzio:
  >>2008.9.25:    update for:Atmeg32 and Free RTOS .......ver3
  Torololve ezen funkciok.
  - A protok open kollekor funkciok lettek megvalositva, a 5V-3.3V
  - Az LCD ajnlott felhuzo ellenalas 330R
ver:4.3 
***************************************************************/

/****************************************************
*
*     Inicializalsi es Felhasznalasi Pelda
* 
******************************************************
//igy kell inicializalni!!
  LcdInit(LCD_FUNC_8B_2L,LCD_ENTMOD_INC,LCD_MODE_DISP); 
  LcdInit(LCD_FUNC_4B_2L,LCD_ENTMOD_INC,LCD_MODE_DISP_CURS_BLINK); 


//igy kell hasznalni a 4-soros LCD-t:
LCDputs("1-Elso Sor \n2-Masodik Sor \n3-Harmadik Sor \n4-Negyedik Sor\n");

!!!!!!!!!!!!!Az utolso sorba is kell egy \n !!!!!!!!!!!!!!!!!!
(igy utoloag lehetett volna a zarokaraktert figyelni...majd legkozelebb...)
*****************************************************/  
/***************************************************************
2013.06.28:
- 
***************************************************************/

#ifndef _HD44780__H_
#define _HD44780__H_ 1

#include <stdint.h>
#include "main.h"

/* Public define -------------------------------------------------------------*/
#define   LCD_LINE1_ADDR              0x00
#define   LCD_LINE2_ADDR              0x40
#define   LCD_DISP_CLEAR              0x01 //Display Clear
#define   LCD_DD_RAM_PTR              0x80
#define   LCD_DISP_CURS_HOME          0x02 //Display / Cursor home
#define   LCD_ENTMOD_INC              0x06 //Entry mode = increment, no shift
#define   LCD_ENTMOD_DEC              0x04 //Entry mode = decrement, no shift
#define   LCD_ENTMOD_INC_SHT          0x07 //Entry mode = increment with shift
#define   LCD_ENTMOD_DEC_SHT          0x05 //Entry mode = decrement with shift
#define   LCD_MODE_DISP_OFF           0x08 //Display OFF 
#define   LCD_MODE_DISP               0x0c //Display ON, no cursor
#define   LCD_MODE_DISP_CURS          0x0e //Display ON, cursor ON, no blinking
#define   LCD_MODE_DISP_CURS_BLINK    0x0f //Display ON, cursor ON, blinking ON
#define   LCD_SHT_CURS_LEFT           0x10 //Shift cursor left
#define   LCD_SHT_CURS_RIGHT          0x14 //Shift cursor right
#define   LCD_SHT_DISP_CURS_LEFT      0x18 //Shift display and cursor to left
#define   LCD_SHT_DISP_CURS_RIGHT     0x1c //Shift display and cursor to right
#define   LCD_FUNC_4B_1L              0x20 //Function set: 4 bits, 1 line
#define   LCD_FUNC_4B_2L              0x28 //Function set: 4 bits, 2 lines
#define   LCD_FUNC_8B_1L              0x30 //Function set: 8 bits, 1 line
#define   LCD_FUNC_8B_2L              0x38 //Function set: 8 bits, 2 lines 
#define   LCD_LINE3_ADDR              0x14 //For 4Line LCD
#define   LCD_LINE4_ADDR              0x54 //For 4LineLCD


/* Public macro --------------------------------------------------------------*/

/* Public functions ----------------------------------------------------------*/


#define LCD_OK  0
#define LCD_BRIGHT_25 3
#define LCD_BRIGHT_50 2
#define LCD_BRIGHT_75 1
#define LCD_BRIGHT_100  0


#define LcdOff()            LcdWriteInstrReg(LCD_MODE_DISP_OFF);
#define LcdCursorEnable()   LcdWriteInstrReg(LCD_MODE_DISP_CURS);
#define LcdCursorDisable()  LcdWriteInstrReg(LCD_MODE_DISP);
#define LcdShiftRight()     LcdWriteInstrReg(LCD_SHT_DISP_CURS_RIGHT);
#define LcdShiftLeft()      LcdWriteInstrReg(LCD_SHT_DISP_CURS_LEFT);

#define LcdCommand(command) LcdWriteInstrReg(command);

uint8_t LcdInit(uint8_t func, uint8_t entmode,uint8_t mode);

void LcdClrscr(void);
void LcdCursorHome(void);

char LcdPutc(char c);
void LcdPuts(const char* string);
void LcdxyPuts(uint8_t x,uint8_t y, const char*string);

void LcdControl(uint8_t value);
void LcdClearLine(uint8_t line);
void Lcdddramaddress(uint8_t DDRAMAddress);
void LcdxyPutc(uint8_t x, uint8_t y, char c);
void LcdxySetCursor(uint8_t x,uint8_t y);
void LcdWriteCGRAM(uint8_t address, const uint8_t *data);
void LcdWriteInstrReg(uint8_t instr);
void LcdPuts(const char *string);
#endif //_HD44780__H_

