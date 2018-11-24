#include "UART.h"
#include <stdint.h>
#include "LPC1114.h"
#include "I2C.h"
#include "UART.h"
#include "startup.h"

#include <stdbool.h>

#include <stdlib.h>

#define FREQ 24000000


int main(void) {
	pll_start(0);
	
	GPIO2DIR |= 1<<6; //led

	//timer enable 
	SYSAHBCLKCTRL |= (1 << 9);
	TMR32B0TCR = 1;

	//I2C_init();
	//UART_init();

	uint32_t x = 0;
	while(1)
	{//blinking led 1s
		if (x!=(TMR32B0TC/24000000))
		{
			GPIO2DATA ^= 1<<6;
			x=(TMR32B0TC/24000000);
		}
	}
}

