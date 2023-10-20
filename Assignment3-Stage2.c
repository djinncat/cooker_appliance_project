/*
 * File:   Assignment3-Stage2.c
 * Author: Kate Bowater
 *
 * Created on May 12, 2023, 7:57 AM
 */


#include <xc.h>
#include "LCD.h"
#include <stdio.h>
#define _XTAL_FREQ 32000000
#pragma config OSC=INTIO67      //use internal oscillator, port A bits 6 and 7 can be used as I/O
#pragma config WDT=OFF

#define SW1 PORTBbits.RB0
#define Up PORTBbits.RB1
#define Down PORTBbits.RB2
#define Select PORTBbits.RB3
#define Back PORTBbits.RB4
#define lidlock LATCbits.LC0
#define ingredient1 LATCbits.LC3
#define ingredient2 LATCbits.LC4
#define ingredient3 LATCbits.LC5

int t, m, h;
unsigned int c = 0;
char str[20], b=0, l;

void initialisation (void);
void lcd_init(void);
void ADC_init(void);
void IO_init(void);
void motor_init(void);

int measure_temp(void);
int measure_mass(void);
int measure_humidity(void);
char buttonstate(void);  
char lid_state (void);

void ingredient_control(void);      //ingredient release control
void lid_lock (void);               //lid lock control
void motorcontrol (int);            //controlling motor speed via potentiometer. Use with motor_test
void motorspeed_0 (void);           //motor 0% duty cycle
void motorspeed_20 (void);          //motor 20% duty cycle
void motorspeed_40 (void);          //motor 20% duty cycle
void motorspeed_60 (void);          //motor 20% duty cycle
void motorspeed_80 (void);          //motor 80% duty cycle
void motorspeed_100 (void);         //motor 100% duty cycle
void motor_test(void);              //for testing purposes, using RA2 pin & pot
void heater_setup (void);
void __interrupt() heater_control (void);
void display(void);

//Initialisation subroutines
void ADC_init(void)            
{    
    ADCON1 = 0b00001011;        //sets bits AN0-3 (RA0-3) as analog for ADC
    ADCON2 = 0b10010010;        //right justified, 4TAD acq time, clock time up to 40MHz
}

void IO_init(void)           
{
    TRISD = 0b00000000;         //portD set as outputs for LCD
    TRISA = 0b11111111;         //portA to input
    TRISB = 0b11111111;         //PortB input for switches
    TRISC = 0b00000000;         //PortC set to output
    LATC = 0b00000000;          //no output PortC on startup
}

void motor_init(void)
{    
    T2CON = 0b00000110;         //enable timer and timer2 preset to 16
    PR2 = 249;                  //minimum 2000Hz with prescaler 16 
    CCP1CON = 0b00001100;       //set PWM mode (CCP1) RC2
    CCPR1L = 0b00000000;        //motor off
}

void initialisation (void)
{
     OSCCON = 0b01110100;        //8MHz internal oscillation frequency
     OSCTUNEbits.PLLEN = 1;      //activate PLL mode, frequency multiplied by 4
     IO_init();
     ADC_init();
     lcd_init();
     motor_init();
     heater_setup();
}


//main program
void main(void) {
    initialisation ();
    
    while(1)
    {       
        __delay_ms(10);
        b = buttonstate();          //b holds the value of the last button pressed
        switch (b)
        {
            case 0: {
                ;
            }
            break;
            case 1: {
                ;
            }
            break;
            case 2: {
                ;
            }
            break;
            case 3: {
                ;
            }
            break;
            case 4: {
                ;
            }
            break;
        }
        /*
        t = measure_temp();       
        m = measure_mass();       
        h = measure_humidity();        
        display(); 

        l = lid_state ();
        motor_test();               //subroutine for testing motor speed control
        lid_lock();                 
        ingredient_control();
        */

    }
}

/************************************************************************
 Functions/Subroutines below here
 ***********************************************************************/
void display (void)                 //LCD display
{
    lcd_cmd(L_L1);
    sprintf(str,"%4d C",t);       //print temperature
    lcd_str(str);
    lcd_cmd(L_RGT);
    sprintf(str,"%4d", h);        //print humidity on same line as temp
    lcd_str(str);
    lcd_cmd(L_RGT);
    lcd_str("%RH");                 
    __delay_ms(100);
    lcd_cmd(L_L2);    
    sprintf(str,"%5d grams", m);      //print mass on second line
    lcd_str(str);
    __delay_ms(100);
}

/*************************************************************************
  * Motor
 * Note: when using motor_test for testing, in PICSIMLAB the switch
 * connected to RA2/TEMP needs to be down to make it work
 *************************************************************************/

void motor_test(void)                    //for testing purposes, uses RA2 pin
{
    int motorvalue;
    float value;
    ADCON0 = 0b00001001;                //channel 2 for analog conversion (RA2)
    ADCON0bits.GODONE = 1;
    while (ADCON0bits.GODONE == 1);
    value = ADRES;
    motorvalue = value / 146.14;      //scaling 1023/7 = 146.14 (6 motor speeds+1)
    motorcontrol (motorvalue);          //sending the value to next routine for changing motor speed
}

void motorcontrol (int motorvalue)      //switch case used to adjust speed based on pot value
{    
    switch (motorvalue)
    {
        case 1: {
            motorspeed_0();
        }
        break;
        case 2: {
            motorspeed_20();
        }
        break;
        case 3: {
            motorspeed_40();
        }
        break;
        case 4: {
            motorspeed_60();
        }
        break;
        case 5:{
            motorspeed_80();
        }
        break;
        case 6: {
            motorspeed_100();
        }
        break;
    }
}

void motorspeed_0 (void)        //0% duty cycle
{
    CCP1CON = 0b00001100;       //bits4-5 two least significant bits = 00
    CCPR1L = 0b00000000;        //MSBs of 0 for 0% duty cycle
}

void motorspeed_20 (void)       //20% duty cycle
{
    CCP1CON = 0b00001100;       //LSBs of decimal 200 is 00
    CCPR1L = 0b00110010;        //most significant bits of decimal number 200
}

void motorspeed_40 (void)       //40% duty cycle
{
    CCP1CON = 0b00001100;       //LSBs of decimal 400 = 00
    CCPR1L = 0b001100100;        //MSBs for number = 400
}

void motorspeed_60 (void)       //60% duty cycle
{
    CCP1CON = 0b00001100;       //LSBs of decimal 600 = 00
    CCPR1L = 0b10010110;        //MSBs for number = 600
}

void motorspeed_80 (void)       //80% duty cycle
{
    CCP1CON = 0b00001100;       //LSBs of decimal 800 = 00
    CCPR1L = 0b11001000;        //MSBs for number = 800
}

void motorspeed_100 (void)      //100% duty cycle
{
    CCP1CON = 0b00001100;           //LSBs of decimal 1000 = 00
    CCPR1L = 0b11111010;        //MSBs of decimal 1000 for 100% duty cycle
}

/*************************************************************************
 * Heater
 * Note: Set up to toggle every 30 seconds. Change to 5 or 10seconds for testing
 *************************************************************************/

void heater_setup (void)
{
  TMR1 = 0;                         //timer 1 cleared
  T1CONbits.TMR1CS = 0;                 //clock source internal for timer mode (Fosc/4)=32MHz/4
  T1CONbits.T1CKPS0 = 0;                //prescaler ratio 1:1
  T1CONbits.T1CKPS1 = 0;                //prescaler ratio 1:1
  LATCbits.LC1 = 0;                     //turn heater off on startup
  T1CONbits.TMR1ON = 1;                 //turn timer1 on
  PIE1bits.TMR1IE = 1;                //timer1 interrupt enable bit
  PIR1bits.TMR1IF = 0;                //timer1 interrupt flag bit cleared
  INTCONbits.PEIE = 1;                //peripheral interrupt bit enabled
  INTCONbits.GIE = 1;                 //global interrupt bit enabled
}

void __interrupt() heater_control (void)              //interrupt routine to toggle heater one/off
{
   if (PIE1bits.TMR1IE && PIR1bits.TMR1IF)           //checking if timer has overflowed.
   {
      c++;                                  //increment c for each overflow
      if(c==610)                            //c=number of overflows. For 30 seconds = c*0.008192 where 0.008192 seconds is time to overflow
      {
        LATCbits.LC1 = ~LATCbits.LC1;           //toggle the heater state
        c = 0;                                  //reset c
      }
      PIR1bits.TMR1IF = 0;                   //timer flag cleared. timer will run again
   }
}

/*************************************************************************
 Solenoids
 *************************************************************************/
void lid_lock (void)
{
    if (PORTAbits.RA7 == 1)         //assigned button on RA7 for testing
    {
        lidlock = ~lidlock;            //toggle lock on/off
        __delay_ms(200);                //button debounce to prevent premature toggle
    }    
}

void ingredient_control (void)
{
    TRISCbits.RC6 = 1;              //for testing purposes. delete when not required
    TRISCbits.RC7 = 1;              //for testing purposes. delete when not required
    if (PORTCbits.RC6 == 1)         //assigned button on RC6 for testing
    {
        ingredient1 = 1;
        __delay_ms(2000);               //holds the dispenser open for 2 seconds before closing
        ingredient1 = 0;
    }
    if (PORTCbits.RC7 == 1)         //assigned button on RC7 for testing
    {
        ingredient2 = 1;
        __delay_ms(2000);
        ingredient2 = 0;
    }
    if (PORTBbits.RB6 == 1)         //assigned button on RB6 for testing
    {
        ingredient3 = 1;
        __delay_ms(2000);
        ingredient3 = 0;
    }
}

/*************************************************************************
 * Button states
 *************************************************************************/
char lid_state (void)
{
    if (SW1 == 0)
    {                        //microswitch detecting lid open or closed
        return (1);
    }
    else
    {
        return (0);
    }
}

char buttonstate(void)
{
    LATB = 0b11111111;      //PortB to high for testing purposes
    if (Up == 0)
    {                       //pushbutton 1
        return (1);
    }
    if (Down == 0)
    {                       //pushbutton 2
        return (2);
    }
    if (Select == 0)
    {                        //pushbutton 3
        return (3);
    }
    if (Back == 0)
    {                       //pushbutton 4
        return (4);
    }
    else
    {
        return(0);
    }
}

/*************************************************************************
 * Temp
 * Mass
 * Humidity
 *************************************************************************/

int measure_temp(void)
{
    float voltage;
    unsigned short tempvalue;
    ADCON0 = 0b00000101;        //channel 1 selected for conversion, a/d converter on
    ADCON0bits.GODONE = 1;      //GO/DONE bit set to 1 -> "A/D conversion in progress"
    while (ADCON0bits.GODONE == 1);   
    tempvalue = ADRES;
    voltage = (float)tempvalue / 204.6;       //scales tempvalue 1023/5V=204.6
    int temp_c = voltage*51.282;            //scale voltage to temperature. 200 deg / 3.9V = 51.282
    return (temp_c);
}

int measure_mass(void)
{
    float voltage;
    unsigned short massvalue;
    ADCON0 = 0b00000001;        //channel 0 selected for conversion, a/d converter on
    ADCON0bits.GODONE = 1;      //GO/DONE bit set to 1 -> "A/D conversion in progress"
    while (ADCON0bits.GODONE == 1);   
    massvalue = ADRES;
    voltage = (float)massvalue / 204.6;       
    int mass = (voltage*(2680/4.99))-680;       //scale voltage to mass. 1V = 0grams
    if (mass <0)
    {
        mass = 0;                           //will not send negative values to display
    }
    return (mass);
}

int measure_humidity(void)          //using this for testing
{
    float voltage;
    unsigned short humidvalue;
    ADCON0 = 0b00001101;            //channel 3 selected for conversion, a/d converter on
    ADCON0bits.GODONE = 1;          //GO/DONE bit set to 1 -> "A/D conversion in progress"
    while (ADCON0bits.GODONE == 1);   
    humidvalue = ADRES;
    voltage = (float)humidvalue / (204.6);       
    int humid = (voltage-0.8)/0.031;         //scale voltage to humidity. 0.8V = 0%RH
    if (humid < 0)
    {
        humid = 0;
    }
    return (humid);
} 


