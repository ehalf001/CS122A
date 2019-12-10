

//--- Servant ---//

#define F_CPU 8000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "usart_ATmega1284.h"
unsigned char data; //PORTA

unsigned char SetBit(unsigned char pin, unsigned char number, unsigned char bin_value)
{
	return (bin_value ? pin | (0x01 << number) : pin & ~(0x01 << number));
}

unsigned char GetBit(unsigned char port, unsigned char number)
{
	return ( port & (0x01 << number) );
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
			if(OCR1B > 55)
			{
				OCR1B = OCR1B - 13;
				_delay_ms(100);
				data = SetBit(data, 2, 0);
			}
		}
		if(GetBit(data, 3))
		{
			if(OCR1B < 295)
			{
				OCR1B = OCR1B + 13;
				_delay_ms(100);
				data = SetBit(data, 3, 0);
			}
		}
	}
	PORTC = GetBit(data, 4) ? SetBit(PORTC, 0, 1) : SetBit(PORTC,0,0);
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
	DDRC = 0xFF; PORTC = 0x00;
	DDRB = 0xFF; PORTB = 0x00;
	DDRD = 0xF0; PORTD = 0x0F;
	TCCR1A |= 1 << WGM11 | 1 << COM1A1 | 1 << COM1B1;
	TCCR1B |= 1 << WGM13 | 1 << WGM12 | 1 << CS10 | 1 << CS11;
	ICR1 = 2499;
	TCNT1 = 0;
	OCR1A = 55; // starts at 0
	OCR1B = 55;	
	initUSART(0);
	USART_Flush(0);
	while (1)
	{
		if(USART_HasReceived(0))
		{
			data = USART_Receive(0);
			USART_Flush(0);
			_delay_ms(100);
		}
		turret_system();
		//PORTB = data;
	}
}


