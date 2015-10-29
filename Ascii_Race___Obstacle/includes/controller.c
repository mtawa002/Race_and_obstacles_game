#ifndef __CONTROLLER__
#define __CONTROLLER__
/*
 * CONTROLLER.c
 *
 *  Author: Johnny Do
 */ 

#include <avr/io.h> 

unsigned char bitVector[16]; //Vector with the value of the buttons depressed
unsigned char ledVector[16]; //Vector with no real purpose

void getControllerData()
{
	unsigned char aCopy;     //Copy of the current input via controller
	int i = 0;               //Index
	PORTA = 0x03;            //Setting the clock and the latch to high. Sets value of buttons to Shift Register
	for(i = 0; i < 16; i++)  //For loop gathering data
	{
		PORTA = 0x01;                     //Setting clock to high in order to shift data over one spot.
		aCopy = ((~PINA) >> 2) & 0x01;    //Buttons have pull-up resistors, so logical NOT values and takes data from PINA 2
		bitVector[i] = aCopy;             //Saving values to global vector
		ledVector[i] = aCopy;             //Saving values to global vector
		PORTA = 0x00;                     //Setting everything to low so that rising edge can trigger shift
	}
	PORTA = 0x01;                         //Setting clock to high when not in use
	
};

/*
Clock Cycle	Button Reported
1	B
2	Y
3	Select
4	Start
5	Up on joypad
6	Down on joypad
7	Left on joypad
8	Right on joypad
9	A
10	X
11	L
12	R
13-16	none (always high)

Pin	Description	Wire Color
1	+5v	White
2	Data Clock	Yellow
3	Data Latch	Orange
4	Serial Data	Red
5	N/C	-
6	N/C	-
7	Ground	Brown
*/

#endif