#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "usart_ATmega1284.h"

#define B1 (~PINB & 0x01)
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

//taken from http://extremeelectronics.co.in/avr-tutorials/using-adc-of-avr-microcontroller/
// void ADC_init()
// {
// 	ADMUX=(1<<REFS1)|(1<<REFS0);                         // For Aref=AVcc;
// 	ADCSRA=(1<<ADEN)|(1<<ADIE)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0); //Rrescalar div factor =128
// }
// uint16_t ReadADC(uint8_t ch){//I added the small delays
// 	//Select ADC Channel ch must be 0-7
// 	ch=ch&0b00000111;
// 	ADMUX|=ch;
// 	_delay_ms(1);
// 	
// 	//Start Single conversion
// 	ADCSRA|=(1<<ADSC);
// 	_delay_ms(1);
// 	
// 	//Wait for conversion to complete
// 	while(!(ADCSRA & (1<<ADIF)));
// 	//Clear ADIF by writing one to it
// 	//Note you may be wondering why we have write one to clear it
// 	//This is standard way of clearing bits in io as said in datasheets.
// 	//The code writes '1' but it result in setting bit to '0' !!!
// 	ADCSRA|=(1<<ADIF);
// 	return(ADC);
// }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
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