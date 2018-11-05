#include "UART.h"
#include <stdint.h>
#include "LPC1114.h"
#include "I2C.h"
#include "UART.h"
#include "INA219.h"
#include "startup.h"

#include <stdbool.h>

#include <string.h>
#include <stdlib.h>

#define FREQ 24000000


enum rState {rsNotStarted,rsNoRW,rsStarted,rsEnded};
#define BUF_SIZE 15
uint8_t cb[BUF_SIZE];
uint8_t ci = 0;
volatile enum rState state = rsNotStarted;
void uartRead(uint8_t c)
{
	if(state == rsNotStarted && c == '|')
	{
		state = rsNoRW;
		ci = 0;
	}
	else if(state == rsNoRW)
	{
		if(c == 'R' || c == 'W')
		{
			cb[ci++] = c;
			state = rsStarted;
		}
		else
		{
			state = rsNotStarted;
			ci = 0;
		}
	}
	else if(state == rsStarted)
	{
		if(c == '|')
		{
			state = rsEnded;
		}
		else if(ci<BUF_SIZE-1)
		{
			cb[ci++] = c;
		}
		else
		{
			state = rsEnded;
		}
	}
}

int main(void) {
	pll_start(0);

	GPIO2DIR |= 1<<6; //led

	//timer enable 
	SYSAHBCLKCTRL |= (1 << 9);
	TMR32B0TCR = 1;

	I2C_init();
	if(1)
	{
		UART_init();
		uint8_t a[] = {'a','e','i','o','u', '\n'};
		UART_write(&a,6);

		UART_read(uartRead);
	}
	uint8_t data[3];
	data[0] = 0x05;;
	data[1] = 0xFF;
	data[2] = 0xFF;
	I2C_write(0x40,data,3); 

	while(1)
	{/*
		if(state == rsEnded)
		{			
			for(uint8_t i=ci;i<BUF_SIZE;i++)
				cb[i] = 0;
			
			uint32_t param = strtol(&(cb[1]),NULL,0);
			
			if(cb[0] == 'R')
			{
				UART_BaseXWrite(INA_readReg(param),16,true);
			}
			else
			{
				uint8_t reg = param>>16;
				uint16_t val = param&0xffff;
				//UART_BaseXWrite(reg,16,true);
				//UART_BaseXWrite(val,16,true);
				INA_writeReg(reg,val);
			}			
			state = rsNotStarted;
		}
		
		if(state == rsNotStarted)
		
			GPIO2DATA = 1<<6;
		else
			GPIO2DATA = 0;		
		*/	
		UART_BaseXWrite(INA_readReg(0x05),16,true);
	}
}

