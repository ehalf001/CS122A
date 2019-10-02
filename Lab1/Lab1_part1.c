/*
Name = Emanuel Halfon
Partner Name = Ivan Lorna
Section 021
*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
//#include "ucr/io.c"
#include <stdio.h>

#define A0 (~PINA & 0x01)

volatile unsigned char TimerFlag = 0; // TimerISR() sets this to 1. C programmer should clear to 0.
volatile unsigned char port_B = 0x00;
unsigned char on_off = 0x00;
// Internal variables for mapping AVR's ISR to our cleaner TimerISR model.
unsigned long _avr_timer_M = 1; // Start count from here, down to 0. Default 1 ms.
unsigned long _avr_timer_cntcurr = 0; // Current internal count of 1ms ticks


const unsigned long PERIOD = 50;

unsigned char SetBit(unsigned char pin, unsigned char number, unsigned char bin_value)
{
	return (bin_value ? pin | (0x01 << number) : pin & ~(0x01 << number));
}





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



const unsigned char tasksSize = 3;
task tasks[3];

enum ToggleButtonStates {Start_On, PressOn, PressOff, On, Off};

int ToggleButton(int state)
{
	switch(state)
	{
		case Start_On:
		state = Off;
		break;
		case On:
		state = A0 ? PressOff : On;
		break;
		case Off:
		state = A0 ? PressOn : Off;
		break;
		case PressOn:
		state =  A0 ? state : On;
		break;
		case PressOff:
		state = A0 ? state : Off;
		break;
		default:
		state = Start_On;
		break;
	}
	switch(state)
	{
		case On:
		on_off = 0x01;
		break;
		case Off:
		on_off = 0x00;
		break;
	}
	return state;
}


enum LightOneStates {Start_one, Light1, Light2, Light3};

int LightStates(int state)
{
	switch(state)
	{
		case Start_one:
		state = on_off ? state : Light1;
		break;
		case Light1:
		state = on_off ? state : Light2;
		break;
		case Light2:
		state = on_off ? state : Light3;
		break;
		case Light3:
		state = on_off ? state : Light1;
		break;
		default:
		state = Start_one;
		break;
	}
	switch(state)
	{
		case Light1:
		port_B = SetBit(port_B,2, 0);
		port_B = SetBit(port_B, 0, 1);
		break;
		case Light2:
		port_B = SetBit(port_B,0,0);
		port_B = SetBit(port_B, 1, 1);
		break;
		case Light3:
		port_B = SetBit(port_B,1,0);
		port_B = SetBit(port_B, 2, 1);
		break;
		default:
		port_B = port_B;
	}
	return state;
}

enum LightOn_OFFStates {Start, LightOn, LightOff};

int LightOn_OFF(int state)
{
	switch(state)
	{
		case Start:
		state = LightOn;
		break;
		case LightOn:
		state = on_off ? state : LightOff;
		break;
		case LightOff:
		state = on_off ? state : LightOn;
		break;
		default:
		state = Start;
	}
	switch(state)
	{
		case LightOn:
		port_B = SetBit(port_B,3,1);
		break;
		case LightOff:
		port_B = SetBit(port_B, 3, 0);
		break;
		default:
		port_B = port_B;
	}
	return state;
}
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


int main(void)
{
	DDRB = 0xFF; PORTB = 0x00;
	DDRA = 0xF0; PORTA = 0x0F;
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xFF; PORTD = 0x00;
	unsigned char i = 0;
	tasks[i].state = Start_one;
	tasks[i].period = 500;
	tasks[i].elapsedTime = 0;
	tasks[i].TickFct = &LightStates;
	i++;
	tasks[i].state = Start;
	tasks[i].period = 1000;
	tasks[i].elapsedTime = 0;
	tasks[i].TickFct = &LightOn_OFF;
	i++;
	tasks[i].state = Start_On;
	tasks[i].period = 50;
	tasks[i].elapsedTime = 0;
	tasks[i].TickFct = &ToggleButton;
	TimerSet(PERIOD);
	TimerOn();
	while(1)
	{
		PORTB = port_B;
	}
	return 0;
}
