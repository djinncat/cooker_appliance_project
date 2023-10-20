/*
 * File:   LCD.c
 * Author: Kate Bowater
 *
 * Created on May 12, 2023, 7:59 AM
 */

#include <xc.h>
#include "LCD.h"
#define _XTAL_FREQ 32000000  // tells compiler of XTAL frequency for delays


void lcd_wr(unsigned char val)
{
  LPORT=val;                //PortD set to value of 'val'
}

void lcd_cmd(unsigned char val)
{
	LENA=1;                 //LCD enabled
        lcd_wr(val);        //function lcd_wr sent the value of 'val'
        LDAT=0;             //bit RE2 set to 0 to take on commands
        __delay_ms(3);
        LENA=0;             //disables LCD
        __delay_ms(3);
	LENA=1;                 //LCD enabled
}
 
void lcd_dat(unsigned char val)
{
	LENA=1;
        lcd_wr(val);
        LDAT=1;
        __delay_ms(3);
        LENA=0;
        __delay_ms(3);
	LENA=1;
}

void lcd_init(void)
{
	TRISEbits.RE1 = 0;      //PortE bit 1 (enable pin) set to output
	TRISEbits.RE2 = 0;      //PortE bit 2 (register select pin) set to output
	TRISD = 0x00;           //PortD set to output
	LENA=0;                 //RE1 enable set to 0 to disable LCD
	LDAT=0;                 //RE2 R/S set to 0 for LCD to take on commands
  __delay_ms(20);
	LENA=1;                 //LCD now enabled
	
	lcd_cmd(L_CFG);         //function lcd_cmd sent value of L_CFG (0x38) to configure LCD
  __delay_ms(5);
	lcd_cmd(L_CFG);
  __delay_ms(1);
	lcd_cmd(L_CFG);         //configuration
	lcd_cmd(L_OFF);         //sends lcd_cmd value of L_OFF (0x08) to turn off display
	lcd_cmd(L_ON);          //sends lcd_cmd value of L_ON (ox0F) to turn on display
	lcd_cmd(L_CLR);         //clear with L_CLR value 0x01
	lcd_cmd(L_CFG);         //configuration of LCD
  lcd_cmd(L_L1);            //value of L_L1 0x80 move to line 1 first character (no clear)
}

void lcd_str(const char* str)
{
 unsigned char i=0;
  
 while(str[i] != 0 )
 {
   lcd_dat(str[i]);
   i++;
 }  
}



