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

#define lid PORTBbits.RB0
#define open 1
#define closed 0
#define B1 PORTBbits.RB1
#define Up 1
#define B2 PORTBbits.RB2
#define Down 2
#define B3 PORTBbits.RB3
#define Select 3
#define B4 PORTBbits.RB4
#define Back 4
#define button_on 0
#define lidlock LATCbits.LC0
#define unlocked 0
#define locked 1
#define dispenser1 LATCbits.LC3
#define dispenser2 LATCbits.LC4
#define dispenser3 LATCbits.LC5
#define alarmsound LATCbits.LC1
#define heater LATCbits.LC6
#define standby 0
#define start 1
#define choice 2
#define soup 3
#define add_water 4
#define add_rice 5
#define add_extra_water 6
#define ready 7
#define cook 8
#define finished 9
#define pause 10
#define error 11
#define chopping 12
#define add_water_soup 13
#define mix 14
#define store_season 15
#define cooking_time 16
#define dispense 17
#define heat_water 18
#define waiting 19
#define add_dry_ingreds 20
#define period 10000   // seconds / (1/ (( FREQ /4) / prescale))
                       //10 ms / (1/ (( 32MHz /4) / 8))

int t, m, h, water_mass, rice_mass, total_mass, soup_mass, mass_var;
unsigned int cook_counter, alert_counter, chop_counter, mix_counter, stir_counter, dispenser_counter, wait_counter, info_counter;
char str[20], button = 0, state = 0, bread_choice, soup_choice, rice_choice;
char x, hold_state, cook_30, cook_60, cook_75, cook_90, cook_120;

void initialisation (void);
void lcd_init(void);
void ADC_init(void);
void IO_init(void);
void motor_init(void);
void timer1_setup (void);

int measure_temp(void);
int measure_mass(void);
int measure_humidity(void);
char buttonstate(void); 

void motorspeed_0 (void);           //motor 0% duty cycle
void motorspeed_20 (void);          //motor 20% duty cycle
void motorspeed_40 (void);          //motor 20% duty cycle
//void motorspeed_60 (void);          //motor 20% duty cycle
void motorspeed_80 (void);          //motor 80% duty cycle
//void motorspeed_100 (void);         //motor 100% duty cycle
void heater_control (void);

void standby_mode (void);

//Initialisation routines
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
    T2CON = 0b00000110;         //enable timer2 preset to 16
    PR2 = 249;                  //minimum 2000Hz with prescaler 16 
    CCP1CON = 0b00001100;       //set PWM mode (CCP1) RC2
    CCPR1L = 0b00000000;        //motor off
}
void standby_mode (void)
{
    lidlock = unlocked;  
    motorspeed_0();            //motor off
    heater = standby;
    lcd_cmd(L_CLR);            //clear the LCD 
}
void initialisation (void)
{
     OSCCON = 0b01110100;        //8MHz internal oscillation frequency
     OSCTUNEbits.PLLEN = 1;      //activate PLL mode, frequency multiplied by 4 - Freq 32MHz
     IO_init();
     ADC_init();
     lcd_init();
     motor_init();
     timer1_setup();
     standby_mode();     
}
/*********************************
 ***********MAIN PROGRAM**********
 ********************************/

void main(void) {
    
    initialisation ();
    
    while(1)
    {       
        //time++;                         //counter to initiate loop time delay 10ms in ISR
        m = measure_mass();             //RA0
        t = measure_temp();             //RA1
        h = measure_humidity();         //RA3
        button = buttonstate();          //holds the last button pressed. RB1-4
        switch (state)
        {
            case standby:                        //state = 0
            {                
                if (button == Select)           
                {
                    state = start;                  //transition out of standby
                }
                else
                {
                  standby_mode ();  
                  state = standby;
                }
            }
            break;
            
            case start:                         //state = 1
            {                                           
                    lcd_cmd(L_L1);
                    lcd_str("Time to cook!   ");                //spaces used to overwrite text rather than clearing entire LCD
                    lcd_cmd(L_L2);
                    lcd_str("Press Up or Down");
                    lcd_cmd(L_L3);
                    lcd_str ("to choose a     ");
                    lcd_cmd(L_L4);
                    lcd_str ("program.        "); 
                    
                if (button == Up || button == Down)
                {
                    soup_choice = 1;                    //the soup option will be the first to display
                    state = choice; 
                }
                
                else if (button == Back)
                {
                    state = standby;                    
                  soup_choice = 0;              //ensuring the choices are 0 to prevent accidentally doubling up
                  rice_choice = 0;
                  bread_choice = 0;
                }                    
            }
            break;
            
            case choice:                        //the user can press up or down button in this state to choose a cooking program
            {
                if (soup_choice == 1)               //display soup on the screen
                {
                    lcd_cmd(L_L1);
                    lcd_str("Choose Program: ");
                    lcd_cmd(L_L2);
                    lcd_str("      Soup      ");        
                    lcd_cmd(L_L3);
                    lcd_str("                ");
                    lcd_cmd(L_L4);
                    lcd_str("                ");                   
                    
                    if (x == 1&& button == Down)             //using x to prevent premature skipping through state
                    {
                        soup_choice = 0;                     //pressing down will remove the soup option and go to bread
                        bread_choice = 1;
                        x = 0;
                    }
                    else if (x == 1 && button == Up)
                    {
                        soup_choice = 0;
                        rice_choice = 1;                    //pressing up will remove soup option and go to rice
                        x = 0;
                    }
                    
                    else if (button == Select)
                    {
                        state = soup;                   //the soup_choice variable stays equal to one so the program follows the soup pathway
                        x = 0;
                    } 
                }
                
                else if (bread_choice == 1)             //display bread option on the screen
                {
                    lcd_cmd(L_L1);
                    lcd_str("Choose Program: ");
                    lcd_cmd(L_L2);
                    lcd_str("     Bread      ");        
                    lcd_cmd(L_L3);
                    lcd_str("                ");
                    lcd_cmd(L_L4);
                    lcd_str("                ");                    
                    
                    if (button == Down)
                    {
                        bread_choice = 0;                   //changes the option from bread to rice
                        rice_choice = 1;
                    }
                    else if (button == Up)
                    {
                        bread_choice = 0;                   //changes the option from bread to soup
                        soup_choice = 1;
                    }
                    
                    else if (button == Select)
                    {
                        state = add_water;                  //pressing select will initiate the bread pathway
                        x = 0;
                    }
                }
                
                else if (rice_choice == 1)                  //display the rice option
                {
                    lcd_cmd(L_L1);
                    lcd_str("Choose Program: ");
                    lcd_cmd(L_L2);
                    lcd_str("      Rice      ");        
                    lcd_cmd(L_L3);
                    lcd_str ("               ");
                    lcd_cmd(L_L4);
                    lcd_str ("               ");
                    
                    if (button == Down)
                    {
                        rice_choice = 0;                    //removes rice and displays soup
                        soup_choice = 1;
                    }
                    else if ( button == Up)
                    {
                        rice_choice = 0;                    //removes rice and displays bread
                        bread_choice = 1;
                    }
                    
                    else if (button == Select)
                    {
                        state = add_water;                  //starts the rice cooking program
                        x = 0;
                    }
                }
                
                if (x == 1 && button == Back)
                {
                    state = start;
                    x = 0;
                }
                else 
                    x=1;   //x=1 is last in this chain so that the program doesn't accidentally skip the state when select is pressed.
            }               //this forces the program to check the button state again before running through the state
            break;
            
            case soup:                  //the 1st step in the soup program
            {    
                    lcd_cmd(L_L1);
                    lcd_str("Add ingredients:");
                    lcd_cmd(L_L2);
                    sprintf(str," %4d grams", m);
                    lcd_str(str);
                    lcd_cmd(L_L3);
                    lcd_str("Press Go when  ");
                    lcd_cmd(L_L4);   
                    lcd_str("ready           ");     
                
                if (button == Select)
                {
                    state = store_season;           //the next state prompts the user to add seasonings to the dispensers
                }
                
                else if (button == Back)
                {
                    state = choice;                 //deselects the cooking program and displays the cooking options again
                }
            }
            break;
                        
            case add_water:                       //this is the 1st step in rice and bread programs
            {         
                lcd_cmd(L_L1);
                lcd_str("Add water to    ");
                lcd_cmd(L_L2);
                lcd_str("cooker:         ");
                lcd_cmd(L_L3);
                sprintf(str," %4d grams", m);       //display change in mass as user adds water
                lcd_str(str);        
                lcd_cmd(L_L4);
                lcd_str("Then press Go   ");  
                
                if (rice_choice == 1 && button == Select)       //is true only for the rice pathway
                {
                    water_mass = m;                 //store the mass of water for the next step
                    state = add_rice;               //next step is to add rice
                }
                else if (bread_choice == 1 && button == Select)     //will only occur for the bread pathway
                {
                   state = store_season;            //next step is to add ingredients to dispensers
                }
                else if (button == Back)
                {
                    state = choice;                 //deselects the program and goes back to the cooking options
                }            
                
            }
            break;
            
            case add_rice:                       //2nd step of the rice program 
            {   
                    lcd_cmd(L_L1);
                    sprintf(str,"Add %4d grams ", water_mass);  //displays the previously stored mass of water
                    lcd_str(str);
                    lcd_cmd(L_L2);
                    lcd_str("of rice:       ");
                    lcd_cmd(L_L3);
                    sprintf(str," %4d grams     ", mass_var);           //displays the changing mass as user adds rice. see below for calculation
                    lcd_str(str);
                    lcd_cmd(L_L4);
                    lcd_str("                "); 
                    
                mass_var = m - water_mass;         //mas_var holds the calculation of the rice mass 
                if (mass_var >= water_mass || button == Select)     //true when the mass of rice is equivalent to the water mass
                {
                    rice_mass = mass_var;              //store the rice mass
                    state = add_extra_water;            //go to the next state
                }
                
                else if (button == Back)
                {                
                    hold_state = add_rice;              //pauses the program and holds the state
                    state = pause;     
                }        
                
            }
            break; 
            
            case store_season:          //the 2nd step of the bread and soup programs
            {
                if(soup_choice ==1)             //only true for the soup pathway
                {
                    lcd_cmd(L_L1);
                    lcd_str("Add seasonings  ");
                    lcd_cmd(L_L2);
                    lcd_str("to dispenser 1. ");
                    lcd_cmd(L_L3);
                    lcd_str("Press Go when   ");
                    lcd_cmd(L_L4);   
                    lcd_str("ready           ");
                    
                    if (lid == open && button == Select)    
                    {
                        hold_state = store_season;          //holds the state before going to the ready state
                        state = ready;                  //goes to the ready state because the lid is open, to prompt the user to close the lid
                    }
                    else if (lid == closed && button == Select)
                    {
                        state = chopping;                    //if the lid is already closed, go to the next state
                    }
                }
                
                else if(bread_choice == 1)              //only true for the bread pathway
                {  
                   lcd_cmd(L_L1);
                    lcd_str("Add yeast, sugar");
                    lcd_cmd(L_L2);
                    lcd_str("and toppings to ");
                    lcd_cmd(L_L3);
                    lcd_str("dispensers. Then");
                    lcd_cmd(L_L4);   
                    lcd_str("press Go        ");
                    
                    if (lid == open && button == Select)
                    {
                        hold_state = store_season;          //goes to the ready state to prompt the user to close the lid
                        state = ready;
                    }
                    else if (lid == closed && button == Select)
                    {
                        state = heat_water;                    //if the lid is already closed, goes to the next step
                    }
                }
                if (button == Back)
                {
                    hold_state = store_season;              //pauses the program and holds the current state
                    state = pause;
                } 
                
            }
            break;
            
            case heat_water:                        //the 3rd step in the bread program
            {
                lcd_cmd(L_L1);
                lcd_str("Heating water   ");
                lcd_cmd(L_L2);
                lcd_str("in progress...  ");
                lcd_cmd(L_L3);
                sprintf(str,"   %3d C        ", t);     //displays the changing temperature
                lcd_str(str);  
                lcd_cmd(L_L4);   
                lcd_str("                ");
                
                if (t >= 38)                        //checking if the temperature is 38 degrees
                {
                    heater = standby;               //if true the heater is off and go to next state
                    state = dispense;               //dispensing ingredients is the next state
                }
                else if (button == Back)            //checking if the back button is pressed to pause the program
                {
                    hold_state = heat_water;        //hold the current state for pausing
                    state = pause;
                }    
                else
                {
                 heater_control();                  //while in this state, run heater control routine
                }
            }
            break;
            
            case chopping:                     //the 3rd step in the soup program 
            {        
                    lcd_cmd(L_L1);
                    lcd_str("Now chopping    ");
                    lcd_cmd(L_L2);
                    lcd_str("ingredients...  ");
                    lcd_cmd(L_L3);
                    lcd_str("                ");
                    lcd_cmd(L_L4);
                    lcd_str("                ");    
                                    
                if (chop_counter >= 1000)         //counter in ISR. 10 sec timer for testing. 1 minute per specs
                {                    
                    soup_mass = m;                              //store the soup's mass to use in the next state
                    motorspeed_0();                             //turn the motor off
                    lidlock = unlocked;                         //unlock lid for the next step
                    state = add_water_soup;                     //change state
                    chop_counter = 0;                           //reset the chop_counter
                }
                
                else if (button == Back)
                {                
                    hold_state = chopping;              //holding the state before going to pause state
                    motorspeed_0();                     //making sure motor is off and lid unlocked for the user
                    lidlock = unlocked;
                    state = pause;
                }
                
                else
                {
                    lidlock = locked;               //ensuring the lid is locked during chopping
                    motorspeed_80();                //PWM motor speed 80%
                    //chop_counter++;               //counter now located in ISR
                }
            }
            break;
            
            case add_water_soup:             //the 4th step in soup program
            { 
                mass_var = m - soup_mass;           //calculating the changing mass
                lcd_cmd(L_L1);
                sprintf(str,"Add %4d grams ", soup_mass);       //display the previously stored mass of soup
                lcd_str(str);
                lcd_cmd(L_L2);
                lcd_str("of water:      ");
                lcd_cmd(L_L3);
                sprintf(str," %4d grams     ", mass_var);           //display the changing mass as user adds water
                lcd_str(str);
                lcd_cmd(L_L4);
                lcd_str("                ");                     
                
                if (mass_var >= soup_mass || button == Select)          //when the water mass reaches the same amount as the original mass
                {
                    state = cooking_time;
                }
                
                else if (button == Back)
                {                
                    hold_state = add_water_soup;                        //pauses the program and holds the state
                    state = pause;    
                }        
            }
            break;
            
            case cooking_time:                      //5th step in soup program. 8th step in bread program
            {                   
                if (soup_choice == 1 && cook_30 == 1)           //cooking time 30 minutes only relevant to soup pathway
                {
                    lcd_cmd(L_L1);
                    lcd_str("Cooking time:   ");
                    lcd_cmd(L_L2);
                    lcd_str("  30 minutes    ");
                    lcd_cmd(L_L3);
                    lcd_str("                ");
                    lcd_cmd(L_L4);
                    lcd_str("                ");
                    
                    if (x == 1 && button == Down)             //using x to prevent premature skipping through state
                    {
                        cook_30 = 0;                        //cook_30 = 30 minutes cooking time
                        cook_60 = 1;                        //pressing down will change cooking time from 30 minutes to 60 minutes
                        x = 0;
                    }
                    else if (x == 1 && button == Up)
                    {
                        cook_30 = 0;                        //pressing up will change cooking time from 30 to 120 mins
                        cook_120 = 1;
                        x = 0;
                    }
                }           

                else if ((soup_choice == 1 || bread_choice == 1) && cook_60 == 1)           //relevant to both soup and bread pathways
                {
                    lcd_cmd(L_L1);
                    lcd_str("Cooking time:   ");
                    lcd_cmd(L_L2);
                    lcd_str("  60 minutes    ");
                    lcd_cmd(L_L3);
                    lcd_str("                ");
                    lcd_cmd(L_L4);
                    lcd_str("                ");
                    
                    if (x == 1 && button == Down)             //using x to prevent premature skipping through state
                    {
                        cook_60 = 0;                        //pressing down changes cook time from 60 to 90 mins
                        cook_90 = 1;
                        x = 0;
                    }
                    else if (bread_choice == 1 && x == 1 && button == Up)       //only relevant to bread pathway
                    {
                        cook_60 = 0;                                            //to change from 60 mins to 75 mins cook time
                        cook_75 = 1;
                        x = 0;
                    }
                    else if (soup_choice == 1 && x == 1 && button == Up)        //only relevant to soup pathway
                    {
                        cook_60 = 0;                                            //to change from 60 mins back to 30 mins
                        cook_30 = 1;
                        x = 0;
                    }
                }
                
                else if (bread_choice == 1 && cook_75 == 1)                     //75 min cook time only relevant to bread pathway
                {
                    lcd_cmd(L_L1);
                    lcd_str("Cooking time:   ");
                    lcd_cmd(L_L2);
                    lcd_str("  75 minutes    ");
                    lcd_cmd(L_L3);
                    lcd_str("                ");
                    lcd_cmd(L_L4);
                    lcd_str("                ");
                    
                    if (x == 1 && button == Down)             //using x to prevent premature skipping through state
                    {
                        cook_75 = 0;                            //pressing down changes cook time from 75 to 60 mins
                        cook_60 = 1;
                        x = 0;
                    }
                    else if (x == 1 && button == Up)
                    {
                        cook_75 = 0;                            //pressing up changes cook time from 75 to 90 mins
                        cook_90 = 1;
                        x = 0;
                    }
                }
                
                else if ((soup_choice == 1 || bread_choice == 1) && cook_90 == 1)       //relevant to both soup and bread pathway
                {
                    lcd_cmd(L_L1);
                    lcd_str("Cooking time:   ");
                    lcd_cmd(L_L2);
                    lcd_str("  90 minutes    ");
                    lcd_cmd(L_L3);
                    lcd_str("                ");
                    lcd_cmd(L_L4);
                    lcd_str("                ");
                    
                    if (soup_choice == 1 && x == 1 && button == Down)             //using x to prevent premature skipping through state
                    {
                        cook_90 = 0;                            //for soup pathway only, change from 90 to 120 mins cook time
                        cook_120 = 1;
                        x = 0;
                    }
                    else if (bread_choice == 1 && x == 1 && button == Down)
                    {
                        cook_90 = 0;                        //for bread pathway only, change from 90 to 75 mins cook time
                        cook_75 = 1;
                        x = 0;
                    }
                    else if (x == 1 && button == Up)
                    {
                        cook_90 = 0;                    //for either soup or bread, change from 90 mins to 60 mins
                        cook_60 = 1;
                        x = 0;
                    } 
                }
                
                else if (cook_120 == 1)             //this will only be be true through the soup pathway if statements
                {
                    lcd_cmd(L_L1);
                    lcd_str("Cooking time:   ");
                    lcd_cmd(L_L2);
                    lcd_str("  120 minutes    ");
                    lcd_cmd(L_L3);
                    lcd_str("                ");
                    lcd_cmd(L_L4);
                    lcd_str("                ");
                    
                    if (x == 1&& button == Down)             //using x to prevent premature skipping through state
                    {
                        cook_120 = 0;                   //change from 120 to 30 mins cook time
                        cook_30 = 1;
                        x = 0;
                    }
                    else if (x == 1 && button == Up)
                    {
                        cook_120 = 0;                   //change from 120 to 90 mins cook time
                        cook_90 = 1;
                        x = 0;
                    }
                }
                
                else if (soup_choice == 1)              //this is the initial display for soup program when none of the other options are true
                {                                       //the user must choose a cooking time
                    lcd_cmd(L_L1);
                    lcd_str("Enough water is ");
                    lcd_cmd(L_L2);
                    lcd_str("added. Press Up ");
                    lcd_cmd(L_L3);
                    lcd_str("or Down to      ");
                    lcd_cmd(L_L4);
                    lcd_str("choose cook time");
                    
                    if (button == Up || button == Down)
                    {
                        cook_30 = 1;                    //after pressing up or down the first option is 30 mins cook time
                    }
                }
                
                else if (bread_choice == 1)             //the default display for the bread program when none of the other if statements are true
                {                                       //the user must choose a cooking time
                    lcd_cmd(L_L1);
                    lcd_str("Press Up or Down");
                    lcd_cmd(L_L2);
                    lcd_str("to select a     ");
                    lcd_cmd(L_L3);
                    lcd_str("cooking time    ");
                    lcd_cmd(L_L4);
                    lcd_str("                ");
                    
                    if (button == Up || button == Down)
                    {
                        cook_60 = 1;                    //the first option is 60 mins cook time
                    }
                }
                
      if ((cook_30||cook_60||cook_75||cook_90||cook_120) && button == Select && lid == closed)  
                    {                               //applicable to both bread and soup programs
                        state = mix;                //once a cooking time is chosen, pressing select with the lid closed will change state
                        lidlock = locked;           //the state mix requires the lid to be locked
                        x = 0;
                    }   
      else if ((cook_30||cook_60||cook_75||cook_90||cook_120) && button == Select && lid == open)
                    {                               //applicable to both bread and soup programs
                        state = ready;              //going to the ready state because the lid is open to prompt user to close lid
                        hold_state = mix;           //mix state requires the lid to be closed for locking
                        x = 0;
                    }
                
                if (x == 1 && button == Back)       //x used to avoid premature change of state
                    {
                        hold_state = cooking_time;      //holds the state and pauses the program
                        state = pause;
                        x = 0;                          //resets x to prevent state skipping
                    }
                else 
                    x=1;                //x needs to be last so the code will check the button state again before running through the case
            }
            break;
            
            case mix:                               //6th and 9th step in soup program. 5th and 9th step in bread program
            {                
                    lcd_cmd(L_L1);
                    lcd_str("Mixing now in   ");
                    lcd_cmd(L_L2);
                    lcd_str("progress...     ");
                    lcd_cmd(L_L3);
                    lcd_str("                ");
                    lcd_cmd(L_L4);
                    lcd_str("                ");
               
                    if (soup_choice == 1)                       //this function only for the soup program
                    {
                        if (mix_counter >= 1000)     //counter in ISR. mix for 20 secs for testing. 5 minutes to meet spec
                        {
                            mix_counter = 0;                        //reset mix_counter (found in ISR)
                            motorspeed_0();                         //turn off motor
                            if (cook_counter == 0)                  //cook_counter is 0 if cooking hasn't been done yet
                            {                                       //so must then go to cooking state
                                state = cook;
                            }
                            else if (cook_counter > 1)              //this is true once cooking is completed
                            {                                       //because this stage in mixing happens after cooking
                              cook_counter = 0;                     //reset the cooking counter
                              state = finished;                     //cooking program is done, go to finish state
                            }
                        }
                        else
                        {
                            lidlock = locked;                //making sure lid is locked while motor is running
                            motorspeed_40();             //motor runs at 40% PWM until the counter in ISR has been reached and change_state = 1
                        }
                    }
                    
                    if (bread_choice == 1 && !(cook_60||cook_75||cook_90))          //for the bread program but before cooking time is chosen
                    {
                        if (mix_counter >= 6000)          //mix time 1 minute
                        {
                            mix_counter = 0;                        //reset the mix counter from ISR
                            motorspeed_0();                         //ensure motor is off
                            state = waiting;
                        }
                        else
                        {
                            lidlock = locked;                //making sure lid is locked while motor is running
                            motorspeed_40();                //motor speed 40%
                        }
                    }
                    else if(bread_choice == 1 && (cook_60||cook_75||cook_90))       //only after a cooking time has been chosen
                    {
                        if (mix_counter >= 6000)          //after 1 min motor speed reduces down
                        {                           
                            motorspeed_20();                                //PWM 20%
                            if (mix_counter >= 6500)     //1.5 minutes for testing. 10 mins needed for specs
                            {
                                motorspeed_0();                             //motor off for the next step
                                state = waiting;
                            }
                        }
                        else
                        {
                            lidlock = locked;                //making sure lid is locked while motor is running
                            motorspeed_40();                //motor speed 40% for 1 minute in bread program
                        }
                    }
                                        
                if (button == Back)
                {
                        hold_state = mix;           //hold the state before pausing
                        lidlock = unlocked;         //unlock the lid for the user
                        motorspeed_0();             //turn off the motor
                        state = pause;
                }
            }
        
            break;
            
            case waiting:                           //6th and 10th step in bread program
            {
                if(!(cook_60||cook_75||cook_90))        //only true is cooking time has not been selected yet
                {
                    lcd_cmd(L_L1);
                    lcd_str("Now waiting for ");
                    lcd_cmd(L_L2);
                    lcd_str("yeast to        ");
                    lcd_cmd(L_L3);
                    lcd_str("react....       ");        
                    lcd_cmd(L_L4);
                    lcd_str("                ");
                
                    if (wait_counter >= 2000)            //20 seconds for testing. wait time 5 minutes per specs
                    {
                        wait_counter = 0;                               //reset the counter
                        water_mass = m;                                 //store the mass in the cooker for comparing in the next step
                        lidlock = unlocked; 
                        state = add_dry_ingreds;   
                    }                
                }
                
                else if (cook_60||cook_75||cook_90)                 //relevant once the cooking time has been chosen
                {
                   lcd_cmd(L_L1);
                    lcd_str("Now waiting for ");
                    lcd_cmd(L_L2);
                    lcd_str("dough to rise...");
                    lcd_cmd(L_L3);
                    lcd_str("                ");        
                    lcd_cmd(L_L4);
                    lcd_str("                ");
                
                    if (wait_counter >= 2000)            //wait 20 secs for testing. 90 minutes for specs
                    {
                        wait_counter = 0;                                   //reset counter
                        state = dispense;
                    }  
                }
                
                if (button == Back)
                {
                    lidlock = unlocked;
                    hold_state = waiting;               //hold the state for pausing the program
                    state = pause;
                }
            }
            break;
            
            case add_dry_ingreds:                   //7th step in bread program
            {
                total_mass = m - water_mass;            //calculating the change in mass
                lcd_cmd(L_L1);
                lcd_str("Add dry         ");
                lcd_cmd(L_L2);
                lcd_str("ingredients to  ");
                lcd_cmd(L_L3);
                lcd_str("cooker:         ");                         
                lcd_cmd(L_L4);
                sprintf(str," %4d grams     ", total_mass);      //displays the change in mass as the user add ingredients
                lcd_str(str);
                
                if (button == Select)
                {
                    state = cooking_time;               //user presses select when done
                }
                if (button == Back)
                {
                    hold_state = add_dry_ingreds;               //hold the state for pausing the program
                    state = pause;
                }
            }
            break;
            
            case add_extra_water:                                   //3rd step in rice program
            {
                int extra_water = m - rice_mass - water_mass;       //temporary variable used for calculation
                    lcd_cmd(L_L1);
                    lcd_str("Enough rice is  ");
                    lcd_cmd(L_L2);
                    lcd_str("added. Add 250g ");
                    lcd_cmd(L_L3);
                    lcd_str("of water:       ");
                    lcd_cmd(L_L4);
                    sprintf(str," %4d grams     ", extra_water);    //displays change in mass as the user add water
                    lcd_str(str);                    
                
                if (extra_water < 0)                
                {
                    extra_water = 0;                //so negative numbers are not displayed
                }                
                
                if (extra_water >= 245 || button == Select)      //after approx 250grams of water added, move on
                {                                        
                    total_mass = m;                         //store the total mass for calculation during cooking
                    hold_state = cook;                      //hold the state and go to the ready state
                    state = ready;                          //to prompt user to close lid
                }   
                
                else if (button == Back)
                {
                    hold_state = add_extra_water;                 //remember the state when paused
                    state = pause;
                }
                
            }
            break;
            
           case ready:                          //4th step in rice program. Also used in other programs                       
            {                                   //to ensure user closes the lid before chopping, cooking, etc
                lcd_cmd(L_L1);
                lcd_str("Please close lid");
                lcd_cmd(L_L2);
                lcd_str("and press Select");
                lcd_cmd(L_L3);
                lcd_str("to continue     ");
                lcd_cmd(L_L4);
                lcd_str("                ");    
                
                if (lid == closed && button == Select)          //when the lid is closed, resume the program from the step it came from
                {                                 
                    lidlock = locked;
                    state = hold_state;                 //hold_state is designated from the step that brought it here
                }
                else if (button == Back)
                {
                    state = pause;      //simply go to pause, hold_state designated in other steps before coming here
                }                
           }
            break;
            
            case cook:                              //7th step in soup program. 5th step in rice program
            {                                       //12th step in bread program
                if(info_counter < 1000)             //this is the normal display
                {
                    lcd_cmd(L_L1);
                    lcd_str("Cooking in      ");
                    lcd_cmd(L_L2);
                    lcd_str("progress...     ");
                    lcd_cmd(L_L3);
                    sprintf(str," %3d C %3d ",t, h);       //display current temperature, humidity
                    lcd_str(str);
                    lcd_str("%RH  "); 
                    lcd_cmd(L_L4);    
                    sprintf(str," %4d grams     ", m);      //display current mass
                    lcd_str(str);
                }                
                if(info_counter >= 1000)        //at 10 seconds, display mass, temp, RH
                {
                    lcd_cmd(L_L3);
                    sprintf(str," %3d C %3d ",t, h);       //display current temperature, humidity
                    lcd_str(str);
                    lcd_str("%RH  "); 
                    lcd_cmd(L_L4);    
                    sprintf(str," %4d grams     ", m);      //display current mass
                    lcd_str(str);
                    if(info_counter >= 2000)              //after another 10 seconds, reset the counter
                    {
                        info_counter = 0; 
                    }
                }
                //cook_counter++;               //now found in ISR
                
                if(rice_choice == 1)            //only relevant for the rice program
                {
                    heater_control();           //run the heater control subroutine to maintain temperature
                    
                    int cook_mass = total_mass - (total_mass*0.25);         //temporary variable calculating drop in mass       
              if (m <= cook_mass || cook_counter >= 3500)     //if mass drops 25% or cooking timer reached
                    {                                               //timer at 10 seconds for testing. 15 minutes for spec
                        heater = standby;               //ensuring heater is off
                        lid = unlocked;                 //unlocking lid
                        cook_counter = 0;               //reset the cooking counter
                        state = finished;               //finalise the cooking program
                    }
                
                    else if (button == Back)
                    {
                        heater = standby;           //turn off the heater and unlock lid when paused
                        lidlock = unlocked;
                        hold_state = cook;                 //remember the state when paused
                        state = pause;
                    }                                         
                }
                
                else if(soup_choice == 1)               //only relevant to the soup program
                {
                    heater_control();                   //run heater subroutine for temp control                    
                    if (stir_counter >= 1000)                 //20 secs for testing. 30 minutes per spec, turn on motor
                    {
                        motorspeed_20();                //stirring at 20% motor speed
                        stir_counter = 0;               //reset the counter so it will run again in 30 minutes
                    }
                    else if(stir_counter >= 500)          //10 secs for testing. stirring occurs for one minute after the motor is on
                    {
                        motorspeed_0();                 //turn off the motor after 1 minute of stirring
                    }                                   //no reset of the counter so it will run again in 30 minutes
                    
if((cook_30 == 1 && cook_counter >= 1000)||(cook_60 == 1 && cook_counter >= 2000)||(cook_90 == 1 && cook_counter >= 2500)||(cook_120 == 1 && cook_counter >= 4000))
                    {    //for soup, if a cooking time is selected and after that time is reached
                        heater = standby;       //turn off the heater
                        motorspeed_0();         //turn off the motor
                        state = dispense;       //next state will dispense the seasonings
                        stir_counter = 0;       //reset the stirring counter
                    }    
                    
                    else if (button == Back)
                    {
                        standby_mode();                     //stops heater, motor, and unlocks lid
                        hold_state = cook;                 //remember the state when paused
                        state = pause;
                    }
                }
                
                else if(bread_choice == 1)              //for the bread pathway
                {
                    heater_control();                   //control the temperature using the subroutine
                    if(cook_60 == 1)                    //applicable for cooking time of 60 minutes
                    {   
                        if (cook_counter >= 2000)     //cooking counter in ISR
                        {                                           //20 seconds for testing. 60 minutes for specs
                            heater = standby;                   //turn the heater off
                            lidlock = unlocked;                 //unlock lid
                            cook_counter = 0;                   //reset cooking counter
                            state = finished;    
                        }                
                        else if (button == Back)
                        {
                            heater = standby;                   //make sure heater is off and lid unlocked for user while paused
                            lidlock = unlocked;
                            hold_state = cook;                 //remember the state when paused
                            state = pause;
                        }
                    }    
                    
                    else if(cook_75 == 1)               //cooking time of 75 minutes selected for bread program
                    {                                   //20 secs for testing, 75 minutes for specs
                        if (cook_counter >= 2000) 
                        {
                            heater = standby;
                            lidlock = unlocked;
                            cook_counter = 0;
                            state = finished;
                        }                
                        else if (button == Back)
                        {
                            heater = standby;
                            lidlock = unlocked;
                            hold_state = cook;                 //remember the state when paused
                            state = pause;
                        }
                    }  
                    
                    else if(cook_90 == 1)                   //when cooking timer of 90 minutes is selected
                    {   
                        if (cook_counter >= 2000) //cooking counter 20 secs for testing. 90 minutes for specs
                        {
                            heater = standby;
                            lidlock = unlocked;
                            cook_counter = 0;
                            state = finished;
                        }                
                        else if (button == Back)
                        {
                            heater = standby;
                            lidlock = unlocked;
                            hold_state = cook;                 //remember the state when paused
                            state = pause;
                        }
                    }   
                }                     
                
                if (alert_counter >= 1000)                   
                {
                    alert_counter = 0;                       //after the time is reached reset the counter and go to the error state 
                    motorspeed_0();
                    heater = standby;
                    lidlock = unlocked;
                    state = error;
                }            
            }
            break;
            
            case dispense:                          //8th step in soup program. 4th and 11th step in bread program
            {            
                lcd_cmd(L_L1);
                lcd_str("Opening         ");
                lcd_cmd(L_L2);
                lcd_str("dispenser.....  ");
                lcd_cmd(L_L3);
                lcd_str("                ");
                lcd_cmd(L_L4);
                lcd_str("                ");                
                
                if (bread_choice == 1 && !(cook_60||cook_75||cook_90))      //only if cooking time has not been selected yet for the bread program
                {
                    dispenser1 = open;                          //open two dispensers for yeast and sugar. Assumed to be preloaded in 1 and 2
                    dispenser2 = open;
                    if (dispenser_counter >= 500 && lid == closed)         //dispensers open for 5 seconds
                    {
                       dispenser1 = closed;                             //close the dispensers
                       dispenser2 = closed;
                       dispenser_counter = 0;                       //reset the counter
                       lidlock = locked;                        //as long as the lid is closed, lock the lid for the next step
                       state = mix;
                    }
                    else if (dispenser_counter >= 500 && lid == open)
                    {
                       dispenser1 = closed;
                       dispenser2 = closed;
                       dispenser_counter = 0;
                       hold_state = dispense;                   //if the lid is open, go the ready state to prompt the user to close the lid
                       state = ready;
                    }
                }
                else if(bread_choice == 1 && (cook_60||cook_75||cook_90))           //once cooking time is chosen, this will become relevant for the bread program
                {
                    dispenser3 = open;                              //the last dispenser to be opened for seasoning or toppings
                    if(dispenser_counter >= 500 && lid == closed)    //open for 5 seconds
                    {
                        dispenser3 = closed;                         //close the dispenser
                        dispenser_counter = 0;                       //reset counter   
                        lidlock = locked;                           //ensure the lid is locked before cooking
                        state = cook;
                    }
                    if(dispenser_counter >= 500 && lid == open)
                    {
                        dispenser3 = closed;                //if the lid is open, go to ready state to prompt the user to close the lid
                        hold_state = dispense;
                        dispenser_counter = 0;
                        state = ready;
                    }
                }
                
                else if (soup_choice == 1)                  //only true for the soup program
                {
                    dispenser1 = open;                      //dispenser 1 opens
                    if (dispenser_counter >= 500 && lid == closed)     //dispenser opens for 5 seconds
                    {                                               
                       dispenser1 = closed;                          //close the dispenser
                       dispenser_counter = 0;                       //reset the counter
                       lidlock = locked;                            //as long as the lid is closed, lock the lid
                       state = mix;                                 //lid locked for the next state
                    }
                    else if (dispenser_counter >= 500 && lid == open)
                    {
                       dispenser1 = closed;
                       dispenser_counter = 0;
                       hold_state = dispense;                   //otherwise if the lid is open, go to the ready state
                       state = ready;                           //to prompt user to close the lid
                    }
                }
                
                if (button == Back)
                {
                    hold_state = dispense;                      //hold the state before pausing
                    lidlock = unlocked;                         //unlocking the lid for the user while paused
                    state = pause;
                }
            }
            break;
            
            case finished:                              //final state for all cooking programs
            {
                if(button >= 1)                     //any button press will go back to standby mode
                {
                    state = standby;                //reset all the counters
                    cook_counter = 0; 
                    alert_counter = 0;
                    stir_counter = 0;
                    mix_counter = 0;
                    chop_counter = 0;
                    dispenser_counter = 0;
                    wait_counter = 0;
                    cook_30 = 0;
                    cook_60 = 0;
                    cook_75 = 0;
                    cook_90 = 0;
                    cook_120 = 0;
                }
                
                else
                {
                lcd_cmd(L_L1);
                lcd_str("    Cooking     ");
                lcd_cmd(L_L2);
                lcd_str("    Finished!   ");
                lcd_cmd(L_L3);
                lcd_str("                ");
                lcd_cmd(L_L4);
                lcd_str("                ");
                }
                lidlock = unlocked;                 //making sure lid is unlocked for user
            }
            break;
            
            case pause:                             //state for pausing the cooking program
            {
                    lcd_cmd(L_L1);
                    lcd_str("Cooking paused. ");
                    lcd_cmd(L_L2);
                    lcd_str("Press Go to     ");
                    lcd_cmd(L_L3);
                    lcd_str("continue or Back");
                    lcd_cmd(L_L4);
                    lcd_str("to quit program."); 
                    
                if (button == Back)             //pressing back button will cancel program and go to standby
                {                               //all counters need to be reset
                    cook_counter = 0; 
                    alert_counter = 0;
                    stir_counter = 0;
                    mix_counter = 0;
                    chop_counter = 0;
                    dispenser_counter = 0;
                    wait_counter = 0;
                    cook_30 = 0;
                    cook_60 = 0;
                    cook_75 = 0;
                    cook_90 = 0;
                    cook_120 = 0;
                    state = standby;
                }
                
                else if (button == Select && (hold_state == cook || hold_state == chopping || hold_state == mix ||hold_state == dispense||hold_state == waiting) && lid == closed)
                {                   //for cooking, chopping and mixing the lid needs to be closed
                    lidlock = locked;           //the lid is locked during these states for safety
                    state = hold_state;         //hold_state will return the user to the most recent step
                }
                
                else if (button == Select && (hold_state == cook || hold_state == chopping || hold_state == mix||hold_state == dispense||hold_state == waiting) && lid == open)
                {                        //if the lid if open when the hold_state is cooking, chopping or mixing 
                    state = ready;       //goes to ready state to prompt the user to close the lid
                }
                
                else if (button == Select)
                {                               //if the most recent state does not require the lid to be closed
                    state = hold_state;         //pressing select will resume the program at the most recent step
                }                
            }
            break;
            
            case error:                             //error state for hazardous cooking conditions
            {
                alarmsound = start;             //turns on the buzzer, pin RC1
                lcd_cmd(L_L1);                  //display error message on LCD
                lcd_str("Error: Weight   ");
                lcd_cmd(L_L2);
                lcd_str("too low or food ");
                lcd_cmd(L_L3);
                lcd_str("too dry. Please ");
                lcd_cmd(L_L4);
                lcd_str("reset cooker.   "); 
            }
            break;

            default:
            {
                lcd_cmd(L_L1);
                lcd_str("Error: no state.");
                lcd_cmd(L_L2);
                lcd_str("Shutdown and try");
                lcd_cmd(L_L3);
                lcd_str("again.          ");
                lcd_cmd(L_L4);
                lcd_str("                "); 
            }
        }       //end of switch case

    }               //end of while loop
}                   //main program bracket
/**************************************
 *******END OF MAIN PROGRAM************
 *************************************/

void heater_control (void)
{
     if (rice_choice == 1 || soup_choice == 1)          //rice and soup share the same cooking temperature
     {
         if (t <= 94)                        //temp required 90 degrees +/- 4
         {                                  //if the temp is too low, heater on
             heater = start;           
         }
         else if (t >= 94)                        //temp too high heater turns off
         {
             heater = standby;
         } 
     }
     else if(bread_choice == 1)                 //only applicable to bread program
     {
         if (state == heat_water)           //only applicable during the heat water state
         {
             if (t < 38)                        //temp required is 38 degrees, heater on
             {
                 heater = start;           
             }         
         }
         else if (state == cook)                //only applicable during cooking state
         {
             if (t <= 194)                        //temp required 190 degrees +/- 4
             {                                   //if temp too low, heater starts
                 heater = start;           
             }
             else if (t >= 194)                        //temp too high heater turns off
             {
                 heater = standby;
             } 
         }
     }
}

/******************************
          INTERRUPT STUFF
 ******************************/
void timer1_setup (void)
{
    T1CONbits.RD16 = 1;
    T1CONbits.T1CKPS0 = 1;                //prescaler ratio 1:8
    T1CONbits.T1CKPS1 = 1;                //prescaler ratio 1:8
    T1CONbits.TMR1CS = 0;
    T1CONbits.TMR1ON = 1;                 //turn timer1 on
    PIR1bits.TMR1IF = 0;                //timer1 interrupt flag bit cleared
    CCP2CON = 0b00001011;
    CCPR2 = period;                 //period is 10000, which gives 10ms for 32MHz
    PIE2bits.CCP2IE = 1;
    INTCONbits.GIE = 1;
    INTCONbits.PEIE = 1;
}

void __interrupt(high_priority) timer_interrupt (void)
{
   if (PIR2bits.CCP2IF == 1)           //checking if timer flag is on (timer overflow)
   {
       if(state == chopping)                //chopping state in soup pathway
       {
           chop_counter++;                  //increment the chop counter for chopping timer
       }
       else if (state == mix)                    //applicable during the mix state
       {
           mix_counter++;                   //increment the mixing counter for timing
       }
       else if(state == dispense)
       {
           dispenser_counter++;                        //counter for timing the dispensers opening
       }
       
       else if (state == cook)                   //applicable during the cooking state
       {
           info_counter++;                  //counter for displaying mass, temp, RH every 10 seconds
           cook_counter++;                   //cooking counter incremented
           stir_counter++;                     //timer for stirring while cooking soup
           if (m < 100 || h <50)                           //hazardous cooking conditions
            {
                 alert_counter++;                            //start the counter
            }
       }
       else if(state == waiting)
       {
           wait_counter++;                     //counter used to time the waiting period
       }
       
       PIR2bits.CCP2IF = 0;                   //timer flag cleared. timer will run again
   }
}

/**********************************************************************
 Functions/Subroutines below here
 **********************************************************************/

/*************************************************************************
  * Motor
 *************************************************************************/
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

//60% duty cycle not used for stage 2
//void motorspeed_60 (void)       //60% duty cycle
//{
//    CCP1CON = 0b00001100;       //LSBs of decimal 600 = 00
//    CCPR1L = 0b10010110;        //MSBs for number = 600
//}

void motorspeed_80 (void)       //80% duty cycle
{
    CCP1CON = 0b00001100;       //LSBs of decimal 800 = 00
    CCPR1L = 0b11001000;        //MSBs for number = 800
}

//100% duty cycle not used for stage 2
//void motorspeed_100 (void)      //100% duty cycle
//{
//    CCP1CON = 0b00001100;           //LSBs of decimal 1000 = 00
//    CCPR1L = 0b11111010;        //MSBs of decimal 1000 for 100% duty cycle
//}

/*************************************************************************
 * Button states
 *************************************************************************/

char buttonstate(void)
{
    //LATB = 0b11111111;      //PortB to high for testing purposes
    
    if (B1 == button_on)
    {                       //pushbutton 1
        return (1);
    }
    else if (B2 == button_on)
    {                       //pushbutton 2
        return (2);
    }
    else if (B3 == button_on)
    {                        //pushbutton 3
        return (3);
    }
    else if (B4 == button_on)
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