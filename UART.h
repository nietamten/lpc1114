#ifndef UART_H
#define UART_H

#include <stdint.h>
#include <stdbool.h>

void UART_init(bool autobaud); 
void UART_write(uint8_t*, uint8_t);
//uint8_t UART_read(void);
uint8_t UART_read(void (*)(uint8_t));

void UART_Handler();

void UART_BaseXWrite(uint64_t n,uint8_t base, bool nl);

uint32_t UART_Read16(char *str);

#endif /* UART_H */



