
#include "UART.h"
#include <stdint.h>
#include "LPC1114.h"
#include "I2C.h"
#include "startup.h"
#include "string.h"

void dec(uint32_t n)
{
	uint8_t d;
	
	if(n==0)
	{
		d='0';
		UART_write(&d,1);
	}
	
	uint8_t num[15];
	memset(num,0,15);
	uint8_t i = 0;
	while(n != 0)
	{
		num[i++]='0'+n%10;
		n=n/10;
	}
	for(i=14;i<15;i--)
	{
		d=num[i];
		if(d!=0)
			UART_write(&d,1);	
	}
	d='\r';
	UART_write(&d,1);
	d='\n';
	UART_write(&d,1);	
}

int main(void) {
	pll_start(0);			// start the PLL

	uint8_t data[] = {'f', '2', 'z','d'};


    GPIO2DIR |= 1<<6;  
    //GPIO2DATA = 1<<6;
	//UART_init();

	//I2C_init();

 		
	//timer enable clk
	SYSAHBCLKCTRL |= (1 << 9);
	TMR32B0TCR = 1;
    	
         	
         	
         	int x=0,y=0;
  /*       	
	
		data[0] = 0x05;
		data[1] = 0x44;
		data[2] = 0x55;
		I2C_write(0x40,data,3);   
		
		data[0] = 0x02;
		I2C_write(0x40,data,1);   
	*/
	while(1) {
		
		if (x!=(TMR32B0TC/24000000))
		{
			   GPIO2DATA ^= 1<<6;	
//			dec(y++);
			x=(TMR32B0TC/24000000);
		}		
		
/*
		I2C_read(0x40,data,2);		
		dec((data[1]|data[0]<<8)>>3);
		
		UART_read(0);
*/	
		
		
		
	//for(int i=0;i<1000000;i++){}
	//LPC_GPIO2->DATA ^= 1<<6;  

		//data[3] = UART_read(0);
/*		
		for(int i=0;i<150000;i++)
		{
		data[0] = 0x00;
		I2C_write(0x40,data,1);
		I2C_read(0x40,data,2);
		}
		GPIO2DATA = 1<<6;
*/	
		//dec(data[0]);
		//dec(data[1]);

//	for(int i=0;1<10000;i++);
//		LPC_GPIO2->DATA ^= 1<<6;
		


	}

}

