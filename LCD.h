/* 
 * File:   LCD.h
 * Author: Kate Bowater
 *
 * Created on May 12, 2023, 7:58 AM
 */


#ifndef LCD_H
#define	LCD_H

#ifdef	__cplusplus
extern "C" {
#endif


#define LENA  PORTEbits.RE1
#define LDAT  PORTEbits.RE2
#define LPORT PORTD


#define L_ON	0x0F     // turn display on
#define L_OFF	0x08     // turn display off
#define L_CLR	0x01     // clear display and home cursor
#define L_L1	0x80     // move to line 1 first character (no clear)
#define L_L2	0xC0     // move to line 2 first character (no clear)
#define L_L3    0x90     // move to line 3 first character (no clear)
#define L_L4    0xD0     // move to line 4 first character (no clear)
#define L_CR	0x0F     // blinking cursor on (display on)		
#define L_NCR	0x0C     // turn cursor off
#define L_LFT	0x10     // move cursor left
#define L_RGT	0x14     // move cursor right

#define L_CFG   0x38     // code for configuring 4 bit mode and other settings

void lcd_init(void);
void lcd_cmd(unsigned char val); 
void lcd_dat(unsigned char val);
void lcd_str(const char* str);
void lcd_wr(unsigned char val);

void soup_program (void);


#ifdef	__cplusplus
}
#endif

#endif	/* LCD_H */

