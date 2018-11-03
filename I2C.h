#ifndef I2C_H
#define I2C_H

#include <stdint.h>
#include "LPC1114.h"

#define I2C_READ    	1
#define I2C_WRITE   	0

#define FALSE 			0
#define TRUE			1

// I2C Task
typedef struct I2CTask {
	uint32_t address;					// I2C slave address
	uint32_t action;					// I2C_READ, I2C_WRITE
	
	volatile uint8_t *buffer;
	uint32_t count;
	uint32_t current;
	uint32_t is_busy;

} I2CTask_t;


void I2C_init(void);
void I2C_write(uint32_t address, uint8_t *buffer, uint32_t count);
void I2C_read(uint32_t address, uint8_t *buffer, uint32_t count, void (*waitF)());
uint8_t I2C_canRead();

void I2C_Handler(void);

#endif /* I2C_H */
