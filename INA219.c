#include "INA219.h"
#include "I2C.h"


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
	data[0] = reg;;
	data[1] = value&0xff;
	data[2] = value>>8;
	I2C_write(0x40,data,3); 	
}

