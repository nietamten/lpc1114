//#include "LPC1114.h"
#include "SysTick.h"
#include "startup.h"
#include "stdint.h"
#include "inc/LPC11xx.h"

volatile uint32_t flag;

void GPIO_callback(uint32_t);
void main_callback(uint32_t);

Timer_t gpio_timer1 = {1000, 0, 0, 0, GPIO_callback};
//Timer_t gpio_timer2 = {1000, 0, 0, 0, GPIO_callback};
//Timer_t main_timer = {1000, 0, 0, 0, main_callback};

void GPIO_callback(uint32_t state) {

	// Toggle GPIO LED
//	GPIO2DATA ^= (1 << PIO2_6);	
    LPC_GPIO2->DATA ^= 1<<6;						
}
/*
void main_callback(uint32_t state) {

	if (flag == 1) {
		// Stop timer 1 and start timer 2	
		SysTick_remove(&gpio_timer1);
		SysTick_add(&gpio_timer2);
		flag = 2;
	} else {
		// Stop timer 2 and start timer 1	
		SysTick_remove(&gpio_timer2);
		SysTick_add(&gpio_timer1);
		flag = 1;
	}
}*/

int main() {

	pll_start(48955000, 48955000);			// start the PLL
	system_init();							// initialize other necessary elements

    LPC_GPIO2->DIR |= 1<<6;    
    LPC_GPIO2->DATA |= 1<<6;    
    
	SysTick_init();
	flag = 1;

	SysTick_add(&gpio_timer1);
	//SysTick_add(&main_timer);
	
	// Despatch timer callbacks if available; or just wait
	SysTick_run();

        return 0;	
}

