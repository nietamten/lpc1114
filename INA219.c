#include "INA219.h"



uint8_t lastReg = 254;

uint16_t INA_readReg(uint8_t reg)
{
	if(lastReg != reg)
	{
		I2C_write(0x40,&reg,1);
		lastReg = reg;
	}
	uint8_t data[] = {0,0};
	I2C_read(0x40,data,2,0);
	return *((uint16_t*)data);
}

void INA_writeReg(uint8_t reg, uint16_t value)
{
	uint8_t data[3];
	data[0] = reg;
	*((uint16_t*)(&data[1])) = 0xfff0;
	
	I2C_write(0x40,data,3); 
}
