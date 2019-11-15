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


int main(void)
{
	DDRA = 0x00; PORTA = 0xFF;
    DDRD = 0xFF; PORTD = 0x00;
	DDRC = 0xFF; PORTC = 0x00;
	TCCR1A |= 1 << WGM11 | 1 << COM1A1;
	TCCR1B |= 1 << WGM13 | 1 << WGM12 | 1 << CS10 | 1 << CS11;
	ICR1 = 2499;
	TCNT1 = 0;
	OCR1A = 55; // starts at 0
	LCD_init();
	LCD_ClearScreen();
	LCD_DisplayString(1,"Degree: ");
	Display_Degree();	
    while (1) 
    {
		/*OCR1A = 55;
		_delay_ms(3000);
		//is at -90 degrees
		OCR1A = 175;
		_delay_ms(3000);
		//is at 0 degrees
		OCR1A = 295;
		_delay_ms(3000);
		//is at 90 degrees
		OCR1A = 110;
		_delay_ms(3000);
		//is at -45 degrees
		OCR1A = 230;
		_delay_ms(3000);
		*/
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
		//Display_Degree();
	}
}

