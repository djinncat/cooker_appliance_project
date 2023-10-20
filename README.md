# cooker_appliance_project
Hi! This code was written as part of an assignment for a hypothetical cooker appliance. Written and tested using MPLABX and simulated using PICSimLab for proof of function.
The appliance entailed several automated cooking programs and was designed to interface with various components such as weight, temperature and humidity sensors.
Below is the full documentation submitted to the university for grading (just an unformatted copy paste job here).

************************************************************************

ELE2303 - Assessment 3

Mixer-Cooker Appliance

Stage 2

Student Name: Kate Bowater
Student Number: U1019160

 
1. Introduction
This report details the software design and implementation of Stage 2 of the Mixer-Cooker Appliance (MCA). State-based programming has been used to develop the code for this stage to meet functional requirements. The design includes a user interface that allows the user to use the four push buttons to choose a cooking program from soup, bread, and rice. The buttons also allow the user to make choices at certain steps within those cooking programs.
The LCD is used throughout the program to provide prompts to the user and display information relevant to the current step in the program. It will also display the sensor values for mass, temperature, and humidity as required. If hazardous conditions are present, the program will be halted and cannot be resumed, and an error message will display relevant to this event.
The program is designed to automate cooking when user input is not required. An interrupt service routine is set up to be used for accurate timing intervals, creating a 10 ms program loop with counters used to create longer delays as required. This creates non-blocking code so the program runs continuously, allowing it to retrieve data to update the LCD with information and to change states when needed.

2. State-Based Program Description
The main program first performs an initialisation routine which goes through a set of subroutines to:
	-set the internal oscillation frequency to 32 MHz
	-set Ports D and C to output , Ports A and B for input
	-enables analog-digital conversion for pins RA0-3
	-initialise the LCD
	-set up PWM on timer 2 for the motor on pin RC2
	-set up interrupt service routine (ISR)

The 10 ms loop timing for the program is generated using timer 1 and CCP2. The value set in the CCP2 register to cause the program to run at 10 ms is calculated using equation 1:
Period=10ms÷1/(32MHz/4*1/8)=10000	(1)

This calculation uses the timer prescaler 1:8. The period of 10000 provided to the CCP register will cause the program to loop every 10 ms. The ISR is then used to increment counters necessary for timing throughout the cooking program.
Once initialisation has completed, the main program enters the continuous while loop. The program runs routines to obtain values for mass, temperature, humidity, and the return value of the subroutine buttonstate() which is the value of the last push button pressed. The four push buttons are assigned names Up, Down, Select, and Back for values 1, 2, 3 and 4 respectively.

Following this, the main program then enters a switch-case which is used for state-based code design with each state designated a name and a value for the switch-case to run.
The state-based code was first planned out using a state diagram. The state diagram, attached as a separate file, contains two images. The first diagram is the initial plan laying out the states required for the predefined steps of the soup, bread and rice cooking programs with an appropriate transition to the next state. While some of the cooking programs share some states, there is duplication in the diagram to demonstrate the flow of each program separately. 
The second image of the state-diagram file includes all states and all transitions in and out of each state including the Pause and Ready states. 

The first state when the MCA is powered on is Standby in which a subroutine standby_mode() is called. This simply ensures the motor and heater are off, the lid is unlocked, and the LCD is blank. When the Go/Select button is pressed (checked via an if statement), it transitions to the Start state which prompts the user to press Up or Down to choose a cooking program. If Up or Down is pressed, the program moves into the Choice state which displays the cooking programs, and the user can choose one to initiate using the Go/Select button. The transition out of Choice to a particular cooking program pathway is dependent on variables that change when Up or Down is pressed. The variables rice_choice, soup_choice, and bread_choice are used as an indication of which program is selected, and are used throughout the code in ‘if’ statements to ensure correct transitions and actions occur corresponding to the program selected, particularly when a state is shared between cooking programs.

Each state also contains code for displaying information on the LCD relating to the current step in the cooking program. It may simply state what the cooker is doing in a step, e.g., mixing, cooking, dispensing seasonings. It may also give instructions to the user, and/or display mass, temperature and humidity. Regardless, during cooking the mass, temperature and humidity are displayed every 10 seconds.

If-else statements are used within each state to check for the conditions required to change state. As mentioned a button press can initiate change which is necessary when user input is required. Other conditions include timing counters (incremented in ISR) reaching a value when longer delays are required for mixing, cooking, and waiting for ingredients to react during the bread program. Other conditions include meeting a particular mass or temperature value before moving onto the next step.
As the program loops, the switch-case will break once the operation in a case has run. This allows the code to return to the start of the while loop obtaining sensor and button values before entering the switch-case again. This produces a non-blocking code so the states can be changed with user input at any time and allows changes in sensor values to be updated on the LCD as required. 

As part of the operational requirements, the code is written so the user can press the Back button to pause the program at any step, then press Go/Select to resume the program from when it was paused or press Back again to cancel the program and return to Standby. The Ready states are included for when a particular step requires the lid to be closed and locked before continuing, such as for cooking and when the blade is turning. The program enters Ready when the lid is open to instruct the user to close the lid before continuing. The program can also be paused then cancelled or resumed from this state.

Upon completion of any cooking program, the code enters a Finished state to indicate to the user that cooking is finished. Any button pressed here will take the program back to Standby.
The Error state is the only state that has no transition out; this is a safety feature and will only occur if the mass is below 100 grams or the humidity is lower than 50% during cooking for 1 minute as this indicates hazardous cooking conditions. If the program enters this state, an error message displays on the LCD and the buzzer is turned on by sending a high signal to pin RC1 which will generate noise to alert the user. The user must then turn off the cooker and start again.
3. New Routine Implementation

As mentioned, the buzzer is required to generate an audible tone in the Error state, and this feature is connected to Port C pin RC1. As the heater was connected to this pin in Stage 1, it will be moved to pin RC6 for Stage 2. The buzzer can be set to active via a switch on the PICSimLab board and will then produce noise when there is high output to the pin. 

A subroutine for controlling the heater is a new implementation. This is called during any step of the cooking program where the heater is required. It uses ‘if’ statements to check for the temperature pertaining to each cooking program and will turn the heater on or off to maintain the required temperature.

The heater interrupt from Stage 1 is also renamed timer_interrupt as this interrupt is no longer needed for the heater specifically. This is now a general ISR where all timing counters are contained.

4. Software Testing Analysis
The testing procedure for the program used the debugging feature on MPLAB X and simulation on PICSimLab. Potentiometers via the spare parts option was used in PICSimLab to simulate the mass, temperature, and humidity sensors. While the program is running, the pin viewer was used to check the high/low states of the pins for the heater and lid sensor. For the lid lock, the Relay 1 LED can be observed; it will turn on when the lid is locked. For time-based requirements, the values for counters are greatly reduced for testing and a stopwatch used to evaluate accuracy.

The following steps outline the testing procedure used during development. The results of final testing were recorded in Table 1:
	-Run cooking program start through finish, mimicking user operation
	-Potentiometers used to alter sensor values as needed
	-Run through multiple times to test different pathways forward
	-Run the program but manually advancing each state by pressing the Go/Select button
	-Testing pause, resume, and cancelling of the cooking program
	-Testing the safety procedure
	-If the mass is lower than 100 grams or humidity is less than 50% for one minute during cooking results in error message and alert noise

Table 1: Cooking Program Testing Checklist
Item to Test	Requirement	Checked
Standby Mode	On startup, equipment is off, lid unlocked, blank display. Pressing Go/Select will exit the state		
Choice of 3 cooking programs	Use Up and Down to view cooking options. Pressing Select will start that program. Pressing Back will deselect program, then pressing again will go to Standby.		
Rice Program	Rice program follows the predefined steps in the specification and state diagram		
Soup Program	Bread program follows the predefined steps in the specification and state diagram		
Bread Program	Bread program follows the predefined steps in the specification and state diagram		
LCD displaying appropriate information	Providing prompts to user relevant to current cooking step.		
	LCD displaying mass, temperature, and humidity every 10 seconds during cooking		
Error state	Occurs only when mass is < 100 grams or humidity < 50% for 1 min during cooking. Error message on LCD. Buzzer turns on. No transition out.		
Button functionality	Pressing Stop/Back will pause the program at any state; pressing Select will resume, pressing Back will cancel the program		
	Pressing Select will manually move onto the next step in the cooking program		
Timers	Accurate timings throughout cooking programs (shortened from specification timing)		
Sensor Values	The scaling and values from sensors display accurately		
PWM	The PWM duty cycles can be differentiated on the oscilloscope (speed can also be evaluated by checking the Cooler bar or fan on the PICSimLab board)		
Notes: To view the PWM on the oscilloscope of PICSimLab, the time base required is 0.1 ms/div.

4.2 Testing Discussion
During testing in the earlier stages of program development, it was found that some states in the switch-case transitioned too quickly - i.e., when Go/Select or Stop/Back is pressed, the state would transition to the next without pause. It was suspected this was due to the speed the program was running, which meant after a transition via a button press the next state would transition while the button is still pressed, causing a premature transition. For example, pressing Select while in the Choice state would transition to the Soup state, but the same button would also cause transition to the state after that, thus essentially skipping the Soup state. This problem also occurred in states that required a choice to be made using the Up or Down buttons; pressing the button would cause the choices to scroll too quickly.

To address this, a variable x is used in the problematic parts of the code to cause the program to loop again, giving time for the user to release or press a different button in time for the program to return the new button value from buttonstate().
During final testing, the program was running noticeably slow. The reason for this is because the code to display to the LCD is run with every loop due to being present within the state. Rather than change the location of all the LCD code in each case, it was quicker to remove the 3 ms delays contained in the LCD.c file for writing to the LCD screen. This drastically improved the program speed.
It should also be noted that the display of mass, temperature, and humidity to the LCD every 10 seconds during cooking is not detectable during normal operation as the code written already displays this information to the user. None-the-less, to ensure the function works, part of the LCD lines were commented out temporarily to allow for visual differentiation which proved to work as intended.
To check the 10 ms loop time based on CCP, a temporary line was written into the ISR for LATC = LATC + 1. Then using the oscilloscope on PICSimLab, the 10 ms interval could be viewed on pin RC0 as seen in Figure 1.
 
Figure 1: demonstration of 10 ms intervals
No other issues were found during final testing of the code.
 
