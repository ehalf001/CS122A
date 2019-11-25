#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "usart_ATmega1284.h"

#define B1 (~PINB & 0x01)

int main(){
	DDRA = 0xFF; PORTA = 0x00;
	DDRB = 0x00; PORTB = 0xFF;
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xFF; PORTD = 0x00;
	unsigned char Master_Slave = (~PINB & 0x02);
	initUSART(0);
	unsigned char data;
	data = 0x01;
	while(1) {
		if(Master_Slave)
		{
			if(B1)
			{
				if(data < 0x80)
				{
					data = data << 1;
				}
				else
				{
					data = 0x01;
				}
				USART_Send(data, 0);
				while(B1);
			}

		}
		else
		{
			if(USART_HasReceived(0))
			{
				data = USART_Receive(0);
				USART_Flush(0);
				_delay_ms(50);
			}
			PORTA = data;
		}
	}
	return 0;
}
