#ifndef UART_H
#define UART_H

#include <stdint.h>

void UART_init(void); 
void UART_write(uint8_t*, uint8_t);
//uint8_t UART_read(void);
uint8_t UART_read(void (*)(uint8_t));

void UART_Handler();

#endif /* UART_H */



