#include "UART.h"
#include <stdint.h>
#include "LPC1114.h"
#include "I2C.h"
#include "UART.h"
#include "INA219.h"
#include "startup.h"

#include <stdbool.h>

#include <stdlib.h>

#define FREQ 24000000


enum rState {rsNotStarted,rsNoRW,rsStarted,rsEnded};
#define BUF_SIZE 25
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
		UART_write((uint8_t*)&a,6);

		UART_read(uartRead);
	}
	
	INA_writeReg(0x05,0x1234);
 
	UART_BaseXWrite(INA_readReg(0x05),16,true);
//	while(1)	UART_BaseXWrite((INA_readReg(0x02)<<3)&0xffff,16,true);
	
	while(1)
	{
		if(state == rsEnded)
		{			
			for(uint8_t i=ci;i<BUF_SIZE;i++)
				cb[i] = 0;
			
			//uint32_t param = strtol((char*)&(cb[1]),NULL,0);
			uint32_t param = UART_Read16((char*)&(cb[1]));
			
			UART_BaseXWrite(param,16,true);
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
				
				//why when it is in INA_writeReg it is not working?!?!
				/*
				uint8_t data[3];
				data[0] = reg;;
				data[1] = val&0xff;
				data[2] = val>>8;
				I2C_write(0x40,data,3); 		
				*/
			}			
			state = rsNotStarted;
		}
		
		if(state == rsNotStarted)		
			GPIO2DATA = 1<<6;
		else
			GPIO2DATA = 0;		
			
		//UART_BaseXWrite(INA_readReg(0x05),16,true);
	}
}

/*
 * 
vals = {};
valNum = 0;

max_current = 2;
current_lsb = max_current/(2^15);
power_lsb = 20*current_lsb;
r_shunt = 0.1;
cal = 0.04096/(current_lsb*r_shunt);
pga = 1; --1 to 4
bvr = 1; --bus voltage range

--[[
ADC4	ADC3	ADC2	ADC1	Mode/Samples	Conversion Time
0		X		0		0		9bit			84 μs		11904 Hz
0		X		0		1		10 bit			148 μs		6757 Hz
0		X		1		0		11 bit			276 μs		3623 Hz
0		X		1		1		12 bit			532 μs		1880 Hz	dafault
1		0		0		0		12 bit			532 μs		1880 Hz
1		0		0		1		2				1.06 ms		943 Hz
1		0		1		0		4				2.13 ms		469 Hz
1		0		1		1		8				4.26 ms		234 Hz
1		1		0		0		16				8.51 ms		117 Hz
1		1		0		1		32				17.02 ms	58 Hz
1		1		1		0		64				34.05 ms	29 Hz
1		1		1		1		128				68.10 ms	15 Hz

PG1	PG0	GAIN	Range
0	0	1		±40 mV
0	1	/2		±80 mV
1	0	/4		±160 mV
1	1	/8		±320 mV  dafault

MODE3	MODE2	MODE1	MODE
0		0		0		Power-down
0		0		1		Shunt voltage, triggered
0		1		0		Bus voltage, triggered
0		1		1		Shunt and bus, triggered
1		0		0		ADC off (disabled)
1		0		1		Shunt voltage, continuous
1		1		0		Bus voltage, continuous
1		1		1		Shunt and bus, continuous	dafault

BRNG
1 = 32V
0 = 16V

SADC = Shunt ADC
BADC = Bus ADC

RST = reset
--]]

function setup(RST,BRNG,SADC4,SADC3,SADC2,SADC1,BADC4,BADC3,BADC2,BADC1,PG1,PG0,MODE3,MODE2,MODE1)
	
	bvr = BRNG;
	
	if PG1 == 0 and PG0 == 0 then
		pga = 1;
	end
	if PG1 == 0 and PG0 == 1 then
		pga = 2;
	end
	if PG1 == 1 and PG0 == 0 then
		pga = 3;
	end
	if PG1 == 1 and PG0 == 1 then
		pga = 4;
	end
				
	conf = 0x00;
	conf = bit32.replace(conf,MODE1,0);
	conf = bit32.replace(conf,MODE2,1);
	conf = bit32.replace(conf,MODE3,2);
	conf = bit32.replace(conf,SADC1,3);
	conf = bit32.replace(conf,SADC2,4);
	conf = bit32.replace(conf,SADC3,5);
	conf = bit32.replace(conf,SADC4,6);
	conf = bit32.replace(conf,BADC1,7);
	conf = bit32.replace(conf,BADC2,8);
	conf = bit32.replace(conf,BADC3,9);
	conf = bit32.replace(conf,BADC4,10);
	conf = bit32.replace(conf,PG0,11);
	conf = bit32.replace(conf,PG1,12);
	conf = bit32.replace(conf,BRNG,13);
	conf = bit32.replace(conf,0,14);
	conf = bit32.replace(conf,RST,15);
	
	tmp1 = bit32.band(conf,0x00FF);
	tmp2 = bit32.band(conf,0xFF00);
	conf = bit32.bor(bit32.lshift(tmp1,8),bit32.rshift(tmp2,8));
	
	z = string.format("%X", conf);
	F.LuaSetRegister("|W0x00"..z.."|",true);
end

function ONinit()
	F.LuaGraphLimit(150);
	
	F.LuaSetRegister("A",true);
	setup(0,1,1,1,1,1,1,1,1,1,0,0,1,1,1);

	z = string.format("%X", cal);
	F.LuaSetRegister("|W0x05"..z.."|",true);	
	F.LuaSetRegister("|R0x05|",true);
	F.LuaSetRegister("|R0x00|",true);
--y = F.LuaSetRegister("|R0x00|",true);	
--	F.PutOnGraph(1,2,3);
--	F.LuaPutOnList("aaa","bbb");
-- this line sets ADC amplification 20x
--	F.LuaPutOnList("r",F.LuaSetRegister("0x00",true));
-- this line sets ADC amplification 1x
--	F.LuaSetRegister(-1,0x27,0x10);

end

function ONread(time)

	y = F.LuaSetRegister("|R0x01|",true);	
	y = tonumber(y,16);
	
	--aaaa?!?
	
	tmp1 = bit32.band(y,0x00FF);
	tmp2 = bit32.band(y,0xFF00);
	y = bit32.bor(bit32.lshift(tmp1,8),bit32.rshift(tmp2,8));
	
	minus = bit32.band(0x8000,y);
	
	y = bit32.replace(y,0,16-pga,pga);
	
	if minus ~= 0 then
		y = bit32.bnot(y);
		y = bit32.band(y,0xffff);
		y = - y;
	end
	
	y = y*0.00001;
	
	F.LuaPutOnList("shunt voltage",y);
	F.PutOnGraph(1,time,y);

----------------------------------------

	y = F.LuaSetRegister("|R0x02|",true);	
	y= tonumber(y,16);
	y=bit32.rshift(y,3);
	if bvr == 0 then
		y = y*2;
	end
	y=y*0.004;
	F.LuaPutOnList("bus voltage",y);
	F.PutOnGraph(2,time,y);
	
----------------------------------------

	y = F.LuaSetRegister("|R0x03|",true);	
	y= tonumber(y,16);
	y=y*power_lsb;
	F.LuaPutOnList("power",y);
	F.PutOnGraph(3,time,y);
	
----------------------------------------

	y = F.LuaSetRegister("|R0x04|",true);	
	y= tonumber(y,16);
	
	tmp1 = bit32.band(y,0x00FF);
	tmp2 = bit32.band(y,0xFF00);
	y = bit32.bor(bit32.lshift(tmp1,8),bit32.rshift(tmp2,8));
		
	minus = bit32.band(0x8000,y);
	if minus ~= 0 then
		y = bit32.band(y,0x7fff);
		y = - y;
	end	
	
	y=y*current_lsb;
	F.LuaPutOnList("current",y);
	F.PutOnGraph(4,time,y);
end
*/
