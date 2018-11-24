#ifndef INA219_H
#define INA219_H
#include <stdint.h>
#include <stdbool.h>

uint16_t	INA_readReg(uint8_t reg);
void		INA_writeReg(uint8_t reg, uint16_t value);

#endif /* INA219_H */
