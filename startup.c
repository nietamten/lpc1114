#include <stdint.h>

#include <stdint.h>

#include "LPC1114.h"

#include "I2C.h"
#include "UART.h"
#include "SysTick.h"


// Sections as defined in the linker script
extern uint32_t __data_load; 
extern uint32_t __data_start;
extern uint32_t __data_end;
extern uint32_t __bss_start;
extern uint32_t __bss_end;
extern uint32_t __ram_end;

// Application entry point main() declaration
int main(void);

void __Default_Handler(void) __attribute__ ((interrupt));
void __Default_Handler(void)
{
    while (1);
}


// Interrupt/Exception Handlers
void Reset_Handler(void);
/*
void SysTick_Handler(void) __attribute__((interrupt, weak, alias("__Default_Handler")));
void I2C_Handler(void) __attribute__ ((weak, alias("__Default_Handler")));
void UART_Handler(void) __attribute__ ((weak, alias("__Default_Handler")));
void GPIO1_Handler(void) __attribute__ ((weak, alias("__Default_Handler")));
*/
// Interrupt Vector at address 0x00000000
void (* __vectors[]) (void) __attribute__ ((section(".vectors"))) = {
	(void (*) (void)) &__ram_end,
	Reset_Handler,		// RESET 
    0,          		// NMI  
    0,          		// Hard Fault  
    0,          		// Reserved 
    0,          		// Reserved 
    0,          		// Reserved 
    0,          		// Reserved 
    0,          		// Reserved 
    0,          		// Reserved 
    0,          		// Reserved 
    0,          		// SVCall  
    0,          		// Reserved 
    0,          		// Reserved 
    0,          		// PendSV  
    SysTick_Handler,    // SysTick  
	0,					// IRQ0
	0,					// IRQ1
	0,					// IRQ2
	0,					// IRQ3
	0,					// IRQ4
	0,					// IRQ5
	0,					// IRQ6
	0,					// IRQ7
	0,					// IRQ8
	0,					// IRQ9
	0,					// IRQ10
	0,					// IRQ11
	0,					// IRQ12
	0,					// IRQ13
	0,					// IRQ14
	I2C_Handler,		// IRQ15
	0,					// IRQ16
	0,					// IRQ17
	0,					// IRQ18
	0,					// IRQ19
	0,					// IRQ20
	UART_Handler,		// IRQ21
	0,					// IRQ22
	0,					// IRQ23
	0,					// IRQ24
	0,					// IRQ25
	0,					// IRQ26
	0,					// IRQ27
	0,					// IRQ28
	0,					// IRQ29
	0,//GPIO1_Handler,		// IRQ30
	0					// IRQ31
};


void Reset_Handler(void) {

	uint32_t *src, *dst;
	
	// Copy .data from Flash to RAM
	src = &__data_load;
	dst = &__data_start;
	while (dst < &__data_end) {
		*dst++ = *src++;
	}
	
	// Clear .BSS section
	dst = &__bss_start;
	while (dst < &__bss_end) {
		*dst++ = 0;
	}
	
	// Call main()
	main();
	
	// Infinite loop
	while(1);

}




void pll_start(uint8_t rc)
{
	if (rc)
	{//RC 24Mhz
		SYSPLLCTRL = 0x01|0x02<<6;
		
		// Power-up PLL block
		PDCONFIG =  (PDCONFIG_SYSPLL_ON << PDCONFIG_SYSPLL_BIT) | (PDCONFIG & ~(1 << PDCONFIG_SYSPLL_BIT));
		
		// Wait for PLL lock 
		while(SYSPLLSTAT != SYSPLLSTAT_LOCKED);
		
		// Select PLL as main clock source
		MAINCLKSEL = MAINCLKSEL_PLLOUT;
		MAINCLKUEN = MAINCLKUEN_OFF;
		MAINCLKUEN = MAINCLKUEN_ON;
	}
	else
	{// '12Mhz crystal' -> 24Mhz clock
		SYSOSCCTRL = 0;			// "lower speed" crystals
		PDCONFIG &= ~(1<<5);	// power-up main oscillator 
		SYSPLLCLKSEL = 1;	// select main oscillator as the input clock for PLL
	
		MAINCLKUEN = MAINCLKUEN_OFF;
		MAINCLKUEN = MAINCLKUEN_ON;

		SYSPLLCTRL = 0x01|0x02<<6;

		PDCONFIG &= ~(1<<7); // power-up PLL 

		while(SYSPLLSTAT != SYSPLLSTAT_LOCKED);	
		
		// Select PLL as main clock source
		MAINCLKSEL = MAINCLKSEL_PLLOUT;
		MAINCLKUEN = MAINCLKUEN_OFF;
		MAINCLKUEN = MAINCLKUEN_ON;
		
		SYSAHBCLKDIV = 1;			

		SYSAHBCLKCTRL |= 1 << SYSAHBCLKCTRL_IOCON_BIT;	// enable clock for IO configuration block
	
	}
}



