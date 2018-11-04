#include "UART.h"
#include <stdint.h>
#include "LPC1114.h"
#include "I2C.h"
#include "startup.h"
#include "string.h"
#include <stdbool.h>

#define FREQ 24000000


uint8_t autosendEn = 0;
volatile uint8_t command = 0;
volatile uint8_t commandCount = 0;
volatile uint32_t commandParam = 0;

uint8_t lastReg = 254;

uint16_t readReg(uint8_t reg, bool waitF)
{
	if(lastReg != reg)
	{
		I2C_write(0x40,&reg,1);
		lastReg = reg;
	}
	uint8_t data[] = {0,0};
	if(waitF)
		I2C_read(0x40,data,2,0);
	else
		I2C_read(0x40,data,2,0);
	return *((uint16_t*)data);
}


void uartRead(uint8_t c)
{
	if(command == 0)
	{
		if((c&0x80) != 0) //not ascii (they must be used for autobaud)
		{
			command = c;
			commandParam = 0;
			commandCount = 0;
		}
	}
	else
	{
		if (commandCount < 4)
		{
			commandParam |= c << (commandCount*8);
			commandCount++;
		}		
	}
}

void processCommand()
{
	if(command != 0)
	{
		uint8_t rw = ((command&0b00000100) != 0);
		uint8_t addr = (command&0b00111000) >> 3;
		uint8_t toLPC = ((command&0b01000000) != 0);
		
		if (rw)
		{
			if (commandCount >3)
			{GPIO2DATA = 1<<6;
				/*if (toLPC)
				{
					if(addr==0)
						autosendEn = commandParam;
				}
				else
				{
					uint8_t data[] = {	addr, 
										commandParam, 
										commandParam << 8,
										commandParam << 16};
					I2C_write(0x40,data,3);
				}*/
				command = 0;
			}
		}
		/*
		else
		{
			if (toLPC)
			{
																																				
			}
			else
			{
				commandParam = readReg(addr,false);
				UART_write((uint8_t*)&commandParam,4);
			}
			command = 0;								
		}
		*/
	}	
}

void autosend()
{
	if(autosendEn)
	{
		for(uint8_t i=0;i<4;i++)
		{
			uint32_t data = readReg(i+1,false);
			UART_write((uint8_t*)&data,4);
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
	UART_init();
	UART_read(uartRead);
	
	while(1)
	{
	//	uartRead(UART_read(0));
		processCommand();
		//autosend();
		
	}

}

