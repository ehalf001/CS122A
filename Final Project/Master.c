// ///--- MASTER ---///
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "usart_ATmega1284.h"
#include "io.c"
unsigned short a, b, c, d, e, f, g, h;
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
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

void ADC_init()
{
	ADMUX=(1<<REFS1)|(1<<REFS0);                         // For Aref=AVcc;
	ADCSRA=(1<<ADEN)|(1<<ADIE)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0); //Rrescalar div factor =128
}
uint16_t ReadADC(uint8_t ch){//I added the small delays
	//Select ADC Channel ch must be 0-7
	ch=ch&0b00000111;
	ADMUX|=ch;
	_delay_ms(1);
	
	//Start Single conversion
	ADCSRA|=(1<<ADSC);
	_delay_ms(1);
	
	//Wait for conversion to complete
	while(!(ADCSRA & (1<<ADIF)));
	//Clear ADIF by writing one to it
	//Note you may be wondering why we have write one to clear it
	//This is standard way of clearing bits in io as said in datasheets.
	//The code writes '1' but it result in setting bit to '0' !!!
	ADCSRA|=(1<<ADIF);
	return(ADC);
}

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
	/*Turn the base of turret*/
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
	if(current_deg < 190 && inc_dec == 0)
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
	if(current_angle < 190 && inc_dec == 0)
	{
		current_angle = current_angle + 10;
	}
	else if (current_angle > 0 && inc_dec == 1)
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


// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
int main(){
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0x00; PORTB = 0xFF;
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xFF; PORTD = 0x00;

	initUSART(0);
	LCD_init();
	LCD_ClearScreen();
	LCD_DisplayString(1,"Degree:         Angle: ");
	current_deg = 0;
	current_angle = 0;
	Display_Degree();
	Display_Angle();
	InterruptEnable();	
	unsigned char i = 0;
	while(1) 
	{
			USART_Send(data, 0);
			Display_Degree();
			Display_Angle();
			_delay_ms(1900);
	}
	return 0;
}