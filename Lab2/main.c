/*    Name & Email: Ivan Lorna / ilorn001@ucr.edu
 *    Partner(s) Name & E-mail: Emanuel Halfon / ehalf001@ucr.edu
 *    Lab Section:  021
 *    Assignment: Lab 2 Part 1
 *    Exercise Description: [optional - include for your own benefit]
 *    
 *    I acknowledge all content contained herein, excluding template 
 *     or example code, is my own original work.
 */

//Includes
#include <avr/io.h>
#include "usart_ATmega1284.h"
#include <avr/interrupt.h>
#include <stdlib.h>
#include <stdio.h>


//Global Variables
unsigned char Master_Servant; //1 if master, 0 is servant
unsigned char poA; //PORTA
volatile unsigned char TimerFlag = 0; // TimerISR() sets this to 1. C programmer should clear to 0.
unsigned long _avr_timer_M = 1; // Start count from here, down to 0. Default 1 ms.
unsigned long _avr_timer_cntcurr = 0; // Current internal count of 1ms ticks
const unsigned long PERIOD = 100; //Preset Period
#define B0 (~PINB & 0x01)

//Functions
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

enum ButtonSignalSender {Start, BSS_Low, BSS_High};
int ButtonSignalSend(int state)
{
	switch(state){//transition

		case Start:
			state = BSS_Low;
			break;
		case BSS_Low:
			state = BSS_High;
			break;
		case BSS_High:
			state = BSS_Low;
			break;
		default:
			state = Start;
	}
	
	switch(state){//action
		case BSS_Low:
			PORTA = 0x00;
			
			break;
		case BSS_High:
			PORTA = 0x01;
			break;
		default:
			poA = poA;
	}
	
	USART_Send(PORTA, 0);
	return state;
}

enum ButtonSignalReciever {Start_S, BSR_Lookout, BSR_High,BSR_High2};
int ButtonSignalRecieve(int state){
	if(USART_HasReceived(0))
	{
		PORTA = USART_Receive(0);
		USART_Flush(0);	
	}
	return state;
}


int main(void)
{
	DDRA = 0xFF; PORTA = 0x00;
	DDRB = 0x00; PORTB = 0xFF;
	DDRC = 0xFF; PORTC = 0x00;

	Master_Servant = B0;
	initUSART(0);
	 
	if(Master_Servant)
	{
		tasks[0].state = Start;
		tasks[0].period = 1000;
		tasks[0].elapsedTime = 0;
		tasks[0].TickFct = &ButtonSignalSend;	
		
	}
	else
	{
		tasks[0].state = Start;
		tasks[0].period = 100;
		tasks[0].elapsedTime = 0;
		tasks[0].TickFct = &ButtonSignalRecieve;	
	}
	
	TimerSet(PERIOD);
	TimerOn();
    while (1) 
    {
		PORTC = B0;
		if(Master_Servant != B0)
		{
			TimerOff();
			Master_Servant = B0;
			if(Master_Servant)
			{
				tasks[0].state = Start;
				tasks[0].period = 1000;
				tasks[0].elapsedTime = 0;
				tasks[0].TickFct = &ButtonSignalSend;
				
			}
			else
			{
				tasks[0].state = Start;
				tasks[0].period = 100;
				tasks[0].elapsedTime = 0;
				tasks[0].TickFct = &ButtonSignalRecieve;
			}
			
			TimerSet(PERIOD);
			TimerOn();
		}
		continue;
    }
}