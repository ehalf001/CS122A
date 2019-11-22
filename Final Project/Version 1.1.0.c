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
#include "io.c"
#include "usart_ATmega1284.h"
#include <avr/interrupt.h>
#include <stdlib.h>
#include <stdio.h>
#include <util/delay.h>

//Global Variables
unsigned char Master_Servant; //1 if master, 0 is servant
unsigned char data; //PORTA
unsigned short current_deg;
unsigned short current_angle;

#define A0 (~PINA & 0x01)
#define A1 (~PINA & 0x02)
#define A2 (~PINA & 0x04)
#define A3 (~PINA & 0x08)
#define A4 (~PINA & 0x10)
#define A5 (~PINA & 0x20)
#define A6 (~PINA & 0x40)
#define A7 (~PINA & 0x80)


//Functions
unsigned char SetBit(unsigned char pin, unsigned char number, unsigned char bin_value)
{
	return (bin_value ? pin | (0x01 << number) : pin & ~(0x01 << number));
}

unsigned char GetBit(unsigned char port, unsigned char number)
{
	return ( port & (0x01 << number) );
}

void InterruptEnable(void)
{
	PCICR |= (1 << PCIE0);
	PCMSK0 |= (1 << PCINT0) | (1 << PCINT1) | (1 << PCINT2) | (1 << PCINT3) | (1 << PCINT4) | (1 << PCINT5) | (1 << PCINT6);
	SREG |= 0x80;                      
}



ISR(PCINT0_vect)
{
	/*Turn the base of turrret*/
	data = A0 ? SetBit(data, 0, 1) : SetBit(data, 0, 0); // Clockwise 
	data = A1 ? SetBit(data, 1, 1) : SetBit(data, 1, 0); // Counterclockwise
	/*Turn Angle of the laser*/
	data = A2 ? SetBit(data, 2, 1) : SetBit(data, 2, 0); // Clockwise
	data = A3 ? SetBit(data, 3, 1) : SetBit(data, 3, 0); // Counterclockwise
	/*Laser*/
	data = A4 ? SetBit(data, 4, 1) : SetBit(data, 4, 0); // Fires
	/*Movement*/
	if(A7)
	{
		if(A5)
		{
			data = SetBit(data, 5, 0);
			data = SetBit(data, 6, 0);
			data = SetBit(data, 7, 1);
		}
		if(A6 && !A5)
		{
			data = SetBit(data, 5, 1);
			data = SetBit(data, 6, 1);
			data = SetBit(data, 7, 1);
		}
	}
	else
	{
		if(A5 && !A6)
		{
			data = SetBit(data, 5, 0);
			data = SetBit(data, 6, 1);
			data = SetBit(data, 7, 1);
		}
		if(A6)
		{
			data = SetBit(data, 5, 1);
			data = SetBit(data, 6, 0);
			data = SetBit(data, 7, 1);
		}
	}
	if(!A5 && !A6)
	{
		data = SetBit(data, 5, 0);
		data = SetBit(data, 6, 0);
		data = SetBit(data, 7, 0);
	}
}

void Display_Degree()
{
	unsigned char inc_dec = GetBit(data, 0) ? 0 : GetBit(data, 1) ? 1 : 2;
	if(current_deg < 180 && inc_dec == 0)
	{
		current_deg = current_deg + 10;
	}
	else if (current_deg > 0 && inc_dec == 1)
	{
		current_deg = current_deg - 10;
	}
	unsigned short degree = current_deg;
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
	if(degree >= 100)
	{
		LCD_Cursor(i);
		LCD_WriteData('0' + (degree / 100));
		i++;
		if(degree == 100)
		{
			LCD_Cursor(i);
			LCD_WriteData('0');
			i++;
		}
		degree = degree - (degree/100)*100;
	}
	if(degree >= 10)
	{
		LCD_Cursor(i);
		LCD_WriteData('0' + (degree / 10));
		i++;
		degree = degree - (degree/10)*10;
	}
	if(degree >= 1)
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

void Display_Angle()
{
	unsigned char inc_dec = GetBit(data, 2) ? 0 : GetBit(data, 3) ? 1 : 2;
	if(current_angle < 135 && inc_dec == 0)
	{
		current_angle = current_angle + 10;
	}
	else if (current_angle > 45 && inc_dec == 1)
	{
		current_angle = current_angle - 10;
	}
	unsigned short angle = current_angle;
	unsigned char i = 24;
	if(angle < 100)
	{
		LCD_Cursor(26);
		LCD_WriteData(' ');
	}
	if(angle < 10)
	{
		LCD_Cursor(25);
		LCD_WriteData(' ');
	}
	if(angle >= 100)
	{
		LCD_Cursor(i);
		LCD_WriteData('0' + (angle / 100));
		i++;
		if(angle == 105)
		{
			LCD_Cursor(i);
			LCD_WriteData('0');
			i++;
		}
		angle = angle - (angle/100)*100;
	}
	if(angle >= 10)
	{
		LCD_Cursor(i);
		LCD_WriteData('0' + (angle / 10));
		i++;
		angle = angle - (angle/10)*10;
	}
	if(angle >= 1)
	{
		LCD_Cursor(i);
		LCD_WriteData('0' + (angle));
		i++;
	}
	else
	{
		LCD_Cursor(i);
		LCD_WriteData('0');
	}
}

void turret_system(void)
{
	if(!(GetBit(data, 0) && GetBit(data, 1)))
	{
		if(GetBit(data, 0))
		{
			if(OCR1A < 295)
			{
				OCR1A = OCR1A + 13;
				_delay_ms(100);
				data = SetBit(data, 0, 0);
			}

		}
		if(GetBit(data, 1))
		{
			if(OCR1A > 55)
			{
				OCR1A = OCR1A - 13;
				_delay_ms(100);
				data = SetBit(data, 1, 0);
			}
		}
	}
	if(!(GetBit(data, 2) && GetBit(data, 3)))
	{
		if(GetBit(data, 2))
		{
			if(OCR1B > 135)
			{
				OCR1B = OCR1B - 15;
				_delay_ms(100);
				data = SetBit(data, 2, 0);
			}
		}
		if(GetBit(data, 3))
		{
			if(OCR1B < 275)
			{
				OCR1B = OCR1B + 15;
				_delay_ms(100);
				data = SetBit(data, 3, 0);
			}
		}
	}
	PORTC = GetBit(data, 4) ? SetBit(PORTC, 0, 1) : SetBit(PORTC, 0, 0);
	if(GetBit(data, 7))
	{
		PORTB = GetBit(data, 5) ? SetBit(SetBit(SetBit(SetBit(PORTB, 0, 1), 1, 1), 2, 0), 3, 0) : SetBit(SetBit(SetBit(SetBit(PORTB, 0, 0), 1, 0), 2, 1), 3, 1);
		PORTB = GetBit(data, 6) ? SetBit(SetBit(SetBit(SetBit(PORTB, 4, 1), 5, 1), 6, 0), 7, 0) : SetBit(SetBit(SetBit(SetBit(PORTB, 4, 0), 5, 0), 6, 1), 7, 1);
	}
	else
	{
		PORTB = 0x00;
	}
}

int main(void)
{
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0xFF; PORTC = 0x00;
	DDRD |= 0xF0; PORTD &= 0x0F;
	data = 0x00;
	Master_Servant = (~PINA & 0x80 ) ? 0x01 : 0x00;
	initUSART(0);
	initUSART(1);
	if(Master_Servant)
	{
		LCD_init();
		LCD_ClearScreen();
		LCD_DisplayString(1,"Degree:         Angle: ");
		current_deg = 0;
		current_angle = 45;
		Display_Degree();
		Display_Angle();
		InterruptEnable();	
	}
	else
	{
		TCCR1A |= 1 << WGM11 | 1 << COM1A1 | 1 << COM1B1;
		TCCR1B |= 1 << WGM13 | 1 << WGM12 | 1 << CS10 | 1 << CS11;
		ICR1 = 2499;
		TCNT1 = 0;
		OCR1A = 55; // starts at 0
		OCR1B = 285;
	
	}
    while (1) 
    {
		if(Master_Servant)
		{
			USART_Send(data, 1);
			PORTB = data;
			Display_Degree();
			Display_Angle();
			_delay_ms(500);		
		}

		else
		{
			if(USART_HasReceived(0))
			{
				data = USART_Receive(0);
				USART_Flush(0);
				turret_system();
			}
			_delay_ms(100);
		}
    }
}
