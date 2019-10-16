/*    Name & Email: Emanuel Halfon / ehalf001@ucr.edu
 *    Partner(s) Name & E-mail: Ivan Lorna / ilorn001@ucr.edu 
 *    Lab Section:  021
 *    Assignment: Lab 3 Part 3
 *    Exercise Description: [optional - include for your own benefit]
 *    
 *    I acknowledge all content contained herein, excluding template 
 *     or example code, is my own original work.
 */

//Includes
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <stdio.h>
#include "SPI.h"
#include "keypad.h"
#include "io.h"
#include "io.c"
#include "bit.h"

//Global Variables
unsigned char count; //Counter
volatile unsigned char TimerFlag = 0; // TimerISR() sets this to 1. C programmer should clear to 0.
unsigned long _avr_timer_M = 1; // Start count from here, down to 0. Default 1 ms.
unsigned long _avr_timer_cntcurr = 0; // Current internal count of 1ms ticks
const unsigned long PERIOD = 50; //Preset Period
unsigned char Pattern = 0x00;
unsigned char dataTransfer = 0x00;
unsigned char Link = 0x01;
//Functions





void TimerOn() {
	// AVR timer/counter controller register TCCR1
	TCCR1B = 0x0B;
	// AVR output compare register OCR1A.
	OCR1A = 125;	// Timer interrupt will be generated when TCNT1==OCR1A
	TIMSK1 = 0x02; // bit1: OCIE1A -- enables compare match interrupt

	//Initialize avr counter
	TCNT1=0;

	_avr_timer_cntcurr = _avr_timer_M;
	SREG |= 0x80; // 0x80: 1000000
}

void TimerOff() {
	TCCR1B = 0x00; // bit3bit1bit0=000: timer off
}


typedef struct Task {
	int state; // Task’s current state
	unsigned long period; // Task period
	unsigned long elapsedTime; // Time elapsed since last task tick
	int (*TickFct)(int); // Task tick function
} task;

const unsigned char tasksSize = 1;
task tasks[1];

void TimerISR()
{
	unsigned char i;
	for (i = 0;i < tasksSize;++i)
	{
		if ((tasks[i].elapsedTime >= tasks[i].period))
		{
			tasks[i].state = tasks[i].TickFct(tasks[i].state);
			tasks[i].elapsedTime = 0;
		}
		tasks[i].elapsedTime += PERIOD;
	}
		
}

ISR(TIMER1_COMPA_vect) {
	// CPU automatically calls when TCNT1 == OCR1 (every 1 ms per TimerOn settings)
	_avr_timer_cntcurr--; // Count down to 0 rather than up to TOP
	if (_avr_timer_cntcurr == 0) { // results in a more efficient compare
		TimerISR(); // Call the ISR that the user uses
		_avr_timer_cntcurr = _avr_timer_M;
	}
}

// Set TimerISR() to tick every M ms
void TimerSet(unsigned long M) {
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}

//State Machines

enum PatternOneStates {Start_One, Low, High};
int PatternOne(int state)
{
	switch(state){//transition
		case Start_One:
			state = Low;
			break;
		case Low:
			state = High;
			break;
		case High:
			state = Low;
			break;
		default:
			state = Start_One;
	}
	
	switch(state){//action
		case Low:
			Pattern = 0xF0;
			break;
		case High:
			Pattern = 0x0F;
			break;
		default:
			Pattern = Pattern;
	}
	return state;
}

enum PatternTwoStates {Start_Two, A_A, f_f,};
int PatternTwo(int state){
	switch(state){//transition
		case Start_Two:
			state = A_A;
			break;
		case A_A:
			state = f_f;
			break;
		case f_f:
			state = A_A;
			break;
		default:
			state = Start_Two;
	}
	
	switch(state){//action
		case Low:
		Pattern = 0xAA;
		break;
		case High:
		Pattern = 0x55;
		break;
		default:
		Pattern = Pattern;
	}
	return state;
}

enum PatternThreeStates {Start_Three, Shift_Down, Shift_Up};
int PatternThree(int state){
	switch(state){//transition
		case Start_Three:
			Pattern = 0x80;
			state = Shift_Down;
			break;
		case Shift_Down:
			state = (Pattern == 0x01) ? Shift_Up : Shift_Down;
			break;
		case Shift_Up:
			state = (Pattern == 0x80) ? Shift_Down : Shift_Up;
			break;
		default:
			state = Start_Three;
	}
	
	switch(state){//action
		case Shift_Down:
			Pattern = Pattern >> 1;
			break;
		case Shift_Up:
			Pattern = Pattern << 1;
			break;
		default:
			Pattern = Pattern;
	}
	return state;
}

enum PatternFourStates {Start_Four, One, Two, Three, Four, Five, Six, Seven, Eight};
int PatternFour(int state){
	switch(state){//transition
		case Start_Four:
			state = One;
			break;
		case One:
			state = Two;
			break;
		case Two:
			state = Three;
			break;
		case Three:
			state = Four;
			break;
		case Four:
			state = Five;
			break;		
		case Five:
			state = Six;
			break;
		case Six:
			state = Seven;
			break;
		case Seven:
			state = Eight;
			break;
		case Eight:
			state = One;
			break;
		default:
			state = Start_Three;
	}
	
	switch(state){//action
		case One:
			Pattern = 0x81;
			break;
		case Two:
			Pattern = 0x42;
			break;
		case Three:
			Pattern = 0x24;
			break;
		case Four:
			Pattern = 0x18;
			break;
		case Five:
			Pattern = 0x24;
			break;
		case Six:
			Pattern = 0x42;
			break;
		case Seven:
			Pattern = 0x81;
			break;
		case Eight:
			Pattern = 0xFF;
			break;
		default:
			Pattern = Pattern;
	}
	return state;
}

void Keypad_decoder(unsigned char x)
{
	switch(x)
	{
		case '\0':  dataTransfer = dataTransfer; break;
		case '1':   dataTransfer = SetBit(dataTransfer, 4, 0);
					dataTransfer = SetBit(dataTransfer, 5, 0);
					dataTransfer = SetBit(dataTransfer, 6, 0);
					break;
		case '2':   dataTransfer = SetBit(dataTransfer, 4, 1);
					dataTransfer = SetBit(dataTransfer, 5, 0);
					dataTransfer = SetBit(dataTransfer, 6, 0);
					break;
		case '3':   dataTransfer = SetBit(dataTransfer, 4, 0);
					dataTransfer = SetBit(dataTransfer, 5, 1);
					dataTransfer = SetBit(dataTransfer, 6, 0);
					break;
		case '4':   dataTransfer = SetBit(dataTransfer, 4, 1);
					dataTransfer = SetBit(dataTransfer, 5, 1);
					dataTransfer = SetBit(dataTransfer, 6, 0);
					break;
		case '5':   dataTransfer = SetBit(dataTransfer, 4, 0);
					dataTransfer = SetBit(dataTransfer, 5, 0);
					dataTransfer = SetBit(dataTransfer, 6, 1);
					break;
		case '6':   dataTransfer = SetBit(dataTransfer, 4, 1);
					dataTransfer = SetBit(dataTransfer, 5, 0);
					dataTransfer = SetBit(dataTransfer, 6, 1);
					break;
		case '7':   dataTransfer = SetBit(dataTransfer, 1, 0);
					dataTransfer = SetBit(dataTransfer, 0, 0);
					break;
		case '8':	dataTransfer = SetBit(dataTransfer, 1, 0);
					dataTransfer = SetBit(dataTransfer, 0, 1);
					break;
		case '9':   dataTransfer = SetBit(dataTransfer, 1, 1);
					dataTransfer = SetBit(dataTransfer, 0, 0);
					break;
		case 'A':   dataTransfer = SetBit(dataTransfer, 3, 0);
					dataTransfer = SetBit(dataTransfer, 2, 0);
					break;
		case 'B':   dataTransfer = SetBit(dataTransfer, 3, 0);
					dataTransfer = SetBit(dataTransfer, 2, 1);
					break;
		case 'C':   dataTransfer = SetBit(dataTransfer, 3, 1);
					dataTransfer = SetBit(dataTransfer, 2, 0);
					break;
		case 'D':   dataTransfer = SetBit(dataTransfer, 3, 1);
					dataTransfer = SetBit(dataTransfer, 2, 1);
					break;
		default:
					dataTransfer = dataTransfer;
	}
}

void LCD_Decoder(unsigned char dataTransfer)
{
		unsigned char SS = 0x00;
		unsigned char patterns = 0x00;
		unsigned char times = 0x00;
		SS = SetBit(SS, 0, GetBit(dataTransfer, 0));
		SS = SetBit(SS, 1, GetBit(dataTransfer, 1));
		patterns = SetBit(patterns, 0, GetBit(dataTransfer, 2));
		patterns = SetBit(patterns, 1, GetBit(dataTransfer, 3));
		times = SetBit(times, 0, GetBit(dataTransfer, 4));
		times = SetBit(times, 1, GetBit(dataTransfer, 5));
		times = SetBit(times, 2, GetBit(dataTransfer, 6));
		LCD_Cursor(7);
		LCD_WriteData('1' + patterns);
		LCD_Cursor(14);
		LCD_WriteData('1' + times);
		LCD_Cursor(21);
		LCD_WriteData('1' + SS);
}

void Data_Decoder(unsigned char dataTransfer)
{
	unsigned char SS = 0x00;
	unsigned char patterns = 0x00;
	unsigned char times = 0x00;
	SS = SetBit(SS, 0, GetBit(dataTransfer, 0));
	SS = SetBit(SS, 1, GetBit(dataTransfer, 1));
	patterns = SetBit(patterns, 0, GetBit(dataTransfer, 2));
	patterns = SetBit(patterns, 1, GetBit(dataTransfer, 3));
	times = SetBit(times, 0, GetBit(dataTransfer, 4));
	times = SetBit(times, 1, GetBit(dataTransfer, 5));
	times = SetBit(times, 2, GetBit(dataTransfer, 6));
	TimerOff();
	switch(patterns)
	{
		case 0:
			tasks[0].state = Start_One;
			tasks[0].TickFct = &PatternOne;
			break;
		case 1:
			tasks[0].state = Start_Two;
			tasks[0].TickFct = &PatternTwo;
			break;
		case 2:
			tasks[0].state = Start_Three;
			tasks[0].TickFct = &PatternThree;
			break; 
		case 3:
			tasks[0].state = Start_Four;
			tasks[0].TickFct = &PatternFour;
			break;
		default:
			break;
	}
	switch(times)
	{
		case 0:
			tasks[0].period = 2000;
			tasks[0].elapsedTime = 0;
			break;
		case 1:
			tasks[0].period = 1000;
			break;
			tasks[0].elapsedTime = 0;
		case 2:
			tasks[0].period = 500;
			break;
			tasks[0].elapsedTime = 0;			
		case 3:
			tasks[0].period = 250;
			break;
			tasks[0].elapsedTime = 0;
		case 4:
			tasks[0].period = 100;
			break;
			tasks[0].elapsedTime = 0;
		case 5:
			tasks[0].period = 50;
			tasks[0].elapsedTime = 0;
			break;
		default:
			break;
	}
	TimerOn();
	
}
/*
void SendData(unsigned char data)
{
	unsigned char i;
	for( i= 0; i < 8; ++i)
	{
		SPI_MasterTransmit(GetBit(data, i));	
	}
}
void RecieveData(unsigned char data)
{
	unsigned char i;
	for( i= 0; i < 8; ++i)
	{
		PORTD = dataTransfer;
		data = SetBit(data, SPI_SlaveReceive(), i);
	}
	PORTD = 0x02;
}*/

int main(void)
{

	DDRA = 0xF0; PORTA = 0x0F;
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xFF; PORTD = 0x00;
	tasks[0].state = Start_One;
	tasks[0].period = 2000;
	tasks[0].elapsedTime = 0;
	tasks[0].TickFct = &PatternOne;
	unsigned char x;
	unsigned char old_data;
	unsigned char SS;
	TimerSet(PERIOD);
	if(Link == 0x00)
	{
		SPI_MasterInit();		
		LCD_init();
		LCD_ClearScreen();
		LCD_DisplayString(1, "Ptrn: 1 Spd: 1  uC: 1");
		TimerOn();
	}
	else
	{
		SPI_SlaveInit();
		TimerOn();
	}
    while (1) 
    {
		SPI_MasterTransmit(dataTransfer);
		old_data = dataTransfer;
		if(Link == 0x00)
		{
			x = GetKeypadKey();
			Keypad_decoder(x);
			if(old_data != dataTransfer)
			{
				LCD_Decoder(dataTransfer);
				//Data_Decoder(dataTransfer);
			}
		}
		else
		{ 
			dataTransfer = SPI_SlaveReceive();
			if(dataTransfer != old_data)
			{
				Data_Decoder(dataTransfer);	
			}
			PORTD = Pattern;
		}
		continue;
    }
}