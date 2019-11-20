/*
 * GccApplication9.c
 *
 * Created: 11/13/2019 2:00:41 PM
 * Author : User
 */ 
#define F_CPU 8000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include "io.c"


#define A0 (~PINA & 0x01)
#define A1 (~PINA & 0x02)
#define A2 (~PINA & 0x04)
#define A3 (~PINA & 0x08)
#define A4 (~PINA & 0x10)

unsigned char data;
void Display_Degree()
{
	unsigned short degree = ((OCR1A - 55)/4)*3;
	unsigned char i = 9;
	if(degree < 100)
	{
		LCD_Cursor(11);
		LCD_WriteData(' ');
	}
	if(degree < 10)
	{
		LCD_Cursor(10);
		LCD_WriteData(' ');
	}
	if(degree > 100)
	{
		LCD_Cursor(i);
		LCD_WriteData('0' + (degree / 100));
		i++;
		degree = degree - (degree/100)*100;
	}
	if(degree > 10)
	{
		LCD_Cursor(i);
		LCD_WriteData('0' + (degree / 10));
		i++;
		degree = degree - (degree/10)*10;
	}
	if(degree > 1)
	{
		LCD_Cursor(i);
		LCD_WriteData('0' + (degree));
		i++;
	}
	else
	{
		LCD_Cursor(i);
		LCD_WriteData('0');
	}
		
}

void Display_Angle(void)
{
	unsigned short degree = (((285 - OCR1B)/14)*9) + 45;
	unsigned char i = 24;
	if(degree < 100)
	{
		LCD_Cursor(26);
		LCD_WriteData(' ');
	}
	if(degree < 10)
	{
		LCD_Cursor(25);
		LCD_WriteData(' ');
	}
	if(degree > 100)
	{
		LCD_Cursor(i);
		LCD_WriteData('0' + (degree / 100));
		i++;
		degree = degree - (degree/100)*100;
	}
	if(degree > 10)
	{
		LCD_Cursor(i);
		LCD_WriteData('0' + (degree / 10));
		i++;
		degree = degree - (degree/10)*10;
	}
	if(degree > 1)
	{
		LCD_Cursor(i);
		LCD_WriteData('0' + (degree));
		i++;
	}
	else
	{
		LCD_Cursor(i);
		LCD_WriteData('0');
	}
}

void InterruptEnable(void)
{
	PCICR |= (1 << PCIE0);
	PCMSK0 |= (1 << PCINT0) | (1 << PCINT1) | (1 << PCINT2) | (1 << PCINT3) | (1 << PCINT4);
	SREG |= 0x80;
}

 ISR(PCINT0_vect)
 {

	if(!(A0 && A1))
	{
		if(A0)
		{
			if(OCR1A < 295)
			{
				OCR1A = OCR1A + 10;
				_delay_ms(100);
				Display_Degree();
			}
			
		}
		if(A1)
		{
			if(OCR1A > 55)
			{
				OCR1A = OCR1A - 10;
				_delay_ms(100);
				Display_Degree();
			}
			
		}
		
		
	}
	if(!(A2 && A3))
	{
		if(A2)
		{
			if(OCR1B > 135)
			{
				OCR1B = OCR1B - 10;
				_delay_ms(100);
				Display_Angle();
			}
		}
		if(A3)
		{
			if(OCR1B < 275)
			{
				OCR1B = OCR1B + 10;
				_delay_ms(100);
				Display_Angle();
			}
		}
	}
	if(A4)
	{
		PORTB = 0x01;
	}
	else
	{
		PORTB = 0x00;
	}
 }

int main(void)
{
	DDRA = 0x00; PORTA = 0xFF;
    DDRD = 0xFF; PORTD = 0x00;
	DDRC = 0xFF; PORTC = 0x00;
	DDRB = 0xFF; PORTB = 0x00;
	TCCR1A |= 1 << WGM11 | 1 << COM1A1 | 1 << COM1B1;
	TCCR1B |= 1 << WGM13 | 1 << WGM12 | 1 << CS10 | 1 << CS11;
	ICR1 = 2499;
	TCNT1 = 0;
	OCR1A = 55; // starts at 0
	OCR1B = 285;
	LCD_init();
	LCD_ClearScreen();
	LCD_DisplayString(1,"Degree:         Angle: ");
	Display_Degree();
	Display_Angle();
	InterruptEnable();	
    while (1) 
    {
		continue;
	}
}

