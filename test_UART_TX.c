#include "UART.h"
#include <stdint.h>
#include "LPC1114.h"
#include "I2C.h"
#include "startup.h"
#include "string.h"
#include <stdbool.h>

#define FREQ 24000000

struct ina
{
	uint8_t i2cAddr;
	//send readed probe count every sec
	uint8_t srpces; 
	
	uint32_t probeCnr;
	uint32_t probeCount;	
	uint32_t probeDelay;
	
	
	//this are bitfields for correspogind ina registers where bits
	// bit 0 - send coutinous probes, 
	// bit 1:2 - 0 average value
	//			 1 min
	//			 2 max
	//			 3 integral
	// bit 3	delay in secs/msecs
	// bit 4:7  delay value (0 - try send all probes)
	uint8_t shuntVoltageCfg;
	uint8_t busVoltageCfg;
	uint8_t powerCfg;
	uint8_t currentCfg;	
	
	//results
	uint32_t shuntVoltage;
	uint32_t busVoltage;
	uint32_t power;
	uint32_t current;
	
	//counters
	uint32_t shuntVoltageCnr;
	uint32_t busVoltageCnr;
	uint32_t powerCnr;
	uint32_t currentCnr;	
	
	//counters last probe time
	uint32_t shuntVoltageCnrLp;
	uint32_t busVoltageCnrLp;
	uint32_t powerCnrLp;
	uint32_t currentCnrLp;	
};

struct ina inas[1];

//=== lpc fake addrs
//1 i2cAddr
//2 srpces
//3 shuntVoltage;
//4 busVoltage;
//5 power;
//6 current;
//7 probe delay;

volatile uint8_t command = 0;
volatile uint8_t commandCount = 0;
volatile uint32_t commandParam = 0;

uint8_t lastReg = 254;

void waitFunction()
{
}

uint16_t readReg(uint8_t reg, bool waitF)
{
	if(lastReg != reg)
	{
		I2C_write(0x40,&reg,1);
		lastReg = reg;
	}
	uint8_t data[] = {0,0};
	if(waitF)
		I2C_read(0x40,data,2,waitFunction);
	else
		I2C_read(0x40,data,2,0);
	return *((uint16_t*)data);
}

void uartRead(uint8_t c)
{
	if(command == 0)
	{
		if(c&0b10000000) //not ascii (they must be used for autobaud)
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
		}		
	}
}

void continousSend(uint8_t cfg,uint32_t *cnr, uint32_t *data)
{
	if (cfg&0b10000000)
	{			
		uint32_t delay = cfg&0b00001111;
		delay *= FREQ;
		if(cfg&0b00010000)
			delay *= FREQ / 1000;

		uint64_t counter = TMR32B0TC;
		if(*cnr>counter)
			counter += 4294967295-*cnr; //2^32-1, when timer overflow
		uint64_t diff = counter - *cnr;
		
		if(diff>delay)
		{
			UART_write((uint8_t*)data,4);
			*data = 0;
			*cnr = TMR32B0TC;
		}
	}
}

void processCommand()
{
	if(command != 0)
	{
		uint8_t rw = ((command&0b00000100) == 0);
		uint8_t addr = (command&0b00111000) >> 3;
		uint8_t toLPC = ((command&0b01000000) == 0);
		
		if (rw)
		{
			if (commandCount >3)
			{
				if (toLPC)
				{
					if(addr==0)
						inas[0].i2cAddr = commandParam;
					if(addr==1)
						inas[0].srpces = commandParam;	
					if(addr==2)
						inas[0].shuntVoltageCfg = commandParam;
					if(addr==3)
						inas[0].busVoltageCfg = commandParam;
					if(addr==4)
						inas[0].powerCfg = commandParam;
					if(addr==5)
						inas[0].currentCfg = commandParam;
					if(addr==6)
						inas[0].probeDelay = commandParam;									
				}
				else
				{
					uint8_t data[] = {	addr, 
										commandParam, 
										commandParam << 8,
										commandParam << 16};
					I2C_write(inas[0].i2cAddr,data,3);
				}
				command = 0;
			}
		}
		else
		{
			if (toLPC)
			{
				if(addr==1)
					UART_write((uint8_t*)&commandParam,4);
				if(addr==2)
					UART_write((uint8_t*)&commandParam,4);
				if(addr==3)
					UART_write((uint8_t*)&commandParam,4);
				if(addr==4)
					UART_write((uint8_t*)&commandParam,4);
				if(addr==5)
					UART_write((uint8_t*)&commandParam,4);
				if(addr==6)
					UART_write((uint8_t*)&commandParam,4);																																									
			}
			else
			{
				commandParam = readReg(addr,false);
				UART_write((uint8_t*)&commandParam,4);
			}
			command = 0;								
		}
	}	
}

void autosend()
{
	continousSend(	0b10010001,
					&inas[0].probeCnr,
					&inas[0].probeCount);
	continousSend(	inas[0].shuntVoltageCfg,
					&(inas[0].shuntVoltageCnr),
					&(inas[0].shuntVoltage));
	continousSend(	inas[0].busVoltageCfg,
					&(inas[0].busVoltageCnr),
					&(inas[0].busVoltage));
	continousSend(	inas[0].powerCfg,
					&(inas[0].powerCnr),
					&(inas[0].power));
	continousSend(	inas[0].currentCfg,
					&(inas[0].currentCnr),
					&(inas[0].current));
}

uint8_t continousProbe(uint8_t cfg,uint32_t *cnr, uint32_t *data,uint8_t reg, uint32_t delay)
{
	if (cfg&0b10000000)
	{			
		uint64_t counter = TMR32B0TC;
		if(*cnr>counter)
			counter += 4294967295-*cnr; //2^32-1, when timer overflow
		uint64_t diff = counter - *cnr;
		
		if(diff>delay)
		{
			commandParam = readReg(reg,1);
			UART_write((uint8_t*)&commandParam,4);
			*data = 0;
			*cnr = TMR32B0TC;
		}
		return 1;
	}
	return 0;
}

uint8_t autoprobe()
{
	uint8_t res=0;
	
	res |=
	continousProbe(	inas[0].shuntVoltageCfg,
					&(inas[0].shuntVoltageCnrLp),
					&(inas[0].shuntVoltage),
					0x01,
					inas[0].probeDelay);
	res |=					
	continousProbe(	inas[0].busVoltageCfg,
					&(inas[0].busVoltageCnrLp),
					&(inas[0].busVoltage),
					0x02,
					inas[0].probeDelay);
	res |=
	continousProbe(	inas[0].powerCfg,
					&(inas[0].powerCnrLp),
					&(inas[0].power),
					0x03,
					inas[0].probeDelay);
	res |=					
	continousProbe(	inas[0].currentCfg,
					&(inas[0].currentCnrLp),
					&(inas[0].current),
					0x04,
					inas[0].probeDelay);
					
	autosend();
	
	return res;
}

void addIna()
{
	inas[0].i2cAddr = 0x40;
	inas[0].srpces = 0;	
	
	inas[0].shuntVoltageCfg = 0;
	inas[0].busVoltageCfg = 0;
	inas[0].powerCfg = 0;
	inas[0].currentCfg = 0;	
	
	inas[0].shuntVoltage = 0;
	inas[0].busVoltage = 0;
	inas[0].power = 0;
	inas[0].current = 0;
	
	inas[0].shuntVoltageCnr = 0;
	inas[0].busVoltageCnr = 0;
	inas[0].powerCnr = 0;
	inas[0].currentCnr = 0;
	
	inas[0].shuntVoltageCnrLp = 0;
	inas[0].busVoltageCnrLp = 0;
	inas[0].powerCnrLp = 0;
	inas[0].currentCnrLp = 0;	
	
	inas[0].probeCount = 0;
	inas[0].probeCnr = 0;
	inas[0].probeDelay = 0;
}

int main(void) {
	pll_start(0);
	
	addIna();
	
	GPIO2DIR |= 1<<6; //led

	//timer enable 
	SYSAHBCLKCTRL |= (1 << 9);
	TMR32B0TCR = 1;

	I2C_init();
	UART_init();
	UART_read(uartRead);
	
	while(1)
	{
	uint8_t d;

		d='0';
		UART_write(&d,1);
		//if(!autoprobe())
		//	processCommand();
	}

}

