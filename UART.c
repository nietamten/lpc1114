#include <stdint.h>
#include "LPC1114.h"
#include "UART.h"
#include "string.h"


void (*UART_callback)(uint8_t);

void UART_init(void) {
	
	// Enable IOCON block
	SYSAHBCLKCTRL |= (1 << SYSAHBCLKCTRL_IOCON_BIT);
	
	// Configure IO Mode and Function of pins 6 and 7 for UART use. FUNC bits (2:0) = 0x01 (RX/TX) respectively.
	IOCON_PIO1_6 = 0x01;		// RX
	IOCON_PIO1_7 = 0x01;		// TX
	
	// Enable power to UART block
	SYSAHBCLKCTRL |= (1 << SYSAHBCLKCTRL_UART_BIT);
	
	// Set transmission parameters to 8N1 (8 data bits, NO partity, 1 stop bit, DLAB=1)
	UART_LCR = (UART_LCR_WORDLENGTH_08 << UART_LCR_WORDLENGTH_BIT) | (1 << UART_LCR_DLAB_BIT);
	
	// Set baud rate to 115200 kb/s @ UART_CLK of 12Mhz  (DLM = 0, DLL = 4, DIVADDVAL = 5, and MULVAL = 8)
	//UART_DLM = 0x00;		// Default
	//UART_DLL = 0x04;
	//UART_FDR = 0x85;		// FDR = (MULVAL << 4 ) | DIVADDVAL

	//UART_DLM = 0x00;		// Default
	//UART_DLL = 0x04;
	//UART_FDR = 1<<4;		
	
	// Set baud rate to 115200 kb/s @ UART_CLK of 48Mhz  (DLM = 0, DLL = 17, DIVADDVAL = 8, and MULVAL = 15)
	// UART_DLM = 0x00;		// Default
	//UART_DLL = 0x11;		
	//UART_FDR = 0xF8;		// FDR = (MULVAL << 4 ) | DIVADDVAL
	
	// Turn off DLAB
	UART_LCR ^= (1 << UART_LCR_DLAB_BIT);
	
	// Enable UART_PCKL devider to supply clock to the baud generator
	UARTCLKDIV = 0x01;
	
	//Autodetect baud rate 	
	UART_ACR |= 0x01;
	while(UART_ACR & 0x01);
	
	// Enable UART Interrupt (NVIC_SETENA is a set only register. No need for a RMW operation)
	NVIC_SETENA = (1 << NVIC_UART_BIT);
}

void UART_write(uint8_t *byte, uint8_t count) {
		
	for (int i = 0; i < count; i++) {
	
		// Send a byte from buffer
		UART_THR = *byte++;
		
		// Wait while THR contains data (until empty)
		while( (UART_LSR & (1 << UART_LSR_THRE_BIT)) == 0 );
	}
	
}

uint8_t UART_read(void (*callback)(uint8_t)) {

	// No callback
	if (callback == 0) {
	
		// Wait until there is data in RBR
		while( (UART_LSR & (1 << UART_LSR_RDA_BIT)) == 0);
	
		return UART_RBR;
	
	} else {
	
		// Attach callback function
		UART_callback = callback;
	
		// Enable UART RX Data Available (RDA) interrupt
		UART_IER = (1 << UART_IER_RDA_BIT);
	}
	return 0;
}

void UART_Handler(void) {

	// Identify and process particular interrupt types
	switch(UART_IIR & UART_IIR_INTID_MASK) {
	
		case (UART_IIR_INTID_LSR << UART_IIR_INTID_BIT):		// Status Change
		
			break;
		
		case (UART_IIR_INTID_RDA << UART_IIR_INTID_BIT):		// RX Data Available
		
			// Call UART read callback function if not NULL
			if (UART_callback)
			{
				while( (UART_LSR & (1 << UART_LSR_RDA_BIT)) != 0)
					UART_callback(UART_RBR);
			}
			
			break;
	}
	
	// Disable UART interrupt
	//UART_IER ^= (1 << UART_IER_RDA_BIT);
}

void UART_BaseXWrite(uint64_t n,uint8_t base, bool nl)
{
	uint8_t d;
	
	if(n==0)
	{
		d='0';
		UART_write(&d,1);
	}
	uint8_t bufflen = 20;
	uint8_t num[bufflen];
	memset(num,0,bufflen);
	uint8_t i = 0;
	while(n != 0)
	{
		num[i++]=(n%base)+1;
		n=n/base;
	}
	for(i=14;i<15;i--)
	{
		d=num[i];
		if(d!=0)
		{
			if(d<11)
				d+='0'-1;
			else
				d='A'+d-11;
			UART_write(&d,1);	
		}
	}
	if(nl)
	{
		d='\r';
		UART_write(&d,1);
		d='\n';
		UART_write(&d,1);	
	}
}
/*
uint64_t UART_BaseXRead(uint8_t *c, uint8_t base)
{
	uint64_t res = 0;
	uint8_t i = 0;
	bool finish = false;
	while(!finish)
	{
		uint8_t ci = c[i++];
		if(toupper(ci) < 10)
		{
		}
	}
}
*/
/* 
uint8_t UART_read(void) {
	
	// Wait until there is data in RBR
	while( (UART_LSR & (1 << UART_LSR_RDA_BIT)) == 0);
	
	return UART_RBR;
}

*/

// Note: How to check if interrupt is happening
	// Wait for interrupt to happen
	//while( (UART_IIR & (1 << UART_IIR_INT_BIT)) == 1 );
	
	//working tty linux configuration for binary transmission
//-ixon -ixoff #software flow control (special characters)
//-isig #special characters
//-icrnl #translate cr to nl
//-iuclc #translate to lowercase
//-ixany #restarting ?
//-imaxbel  #beep
//-iutf8
// -icanon #special characters
// -iexten #special characters
// -echo #...
