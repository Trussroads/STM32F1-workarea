#include <stm32f10x.h>

void Delay(__IO uint32_t nCount) {
  while(nCount--) {
  }
}

/* This funcion shows how to initialize 
 * the GPIO pins on GPIOC / GPIOA and how to configure
 * them as inputs and outputs 
 */
void GPIO_init(void){
	
	/* This TypeDef is a structure defined in the
	 * ST's library and it contains all the properties
	 * the corresponding peripheral has, such as output mode,
	 * pullup / pulldown resistors etc.
	 * 
	 * These structures are defined for every peripheral so 
	 * every peripheral has it's own TypeDef. The good news is
	 * they always work the same so once you've got a hang
	 * of it you can initialize any peripheral.
	 * 
	 * The properties of the periperals can be found in the corresponding
	 * header file e.g. stm32f10x_gpio.h and the source file stm32f10x_gpio.c
	 */
	GPIO_InitTypeDef GPIO_InitStruct;
	
	/* This enables the peripheral clock to the GPIOD IO module
	 * Every peripheral's clock has to be enabled 
	 * 
	 * The STM32F10x documentation file (*.chm) and
	 * datasheet contain the information which peripheral clock has to be used.
	 * 
	 * It is also mentioned at the beginning of the peripheral library's 
	 * source file, e.g. stm32f10x_gpio.c
	 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	
	/* In this block of instructions all the properties
	 * of the peripheral, the GPIO port in this case,
	 * are filled with actual information and then 
	 * given to the Init function which takes care of 
	 * the low level stuff (setting the correct bits in the 
	 * peripheral's control register)
	 * PC8 and PC9
	 */
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_8; // we want to configure all LED GPIO pins
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP; 	// we want the pins to be a push-pull output
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz; 	// this sets the GPIO modules clock speed
	GPIO_Init(GPIOC, &GPIO_InitStruct); 			// this finally passes all the values to the GPIO_Init function which takes care of setting the corresponding bits.
		
	/* Here the GPIOD module is initialized.
	 * We want to use PA0 as an input because
	 * the USER button on the board is connected
	 * between this pin and VCC.
	 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;		  // we want to configure PA0
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPD; 	  // we want it to be an input with a pull-down resistor enabled
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;//this sets the GPIO modules clock speed
	GPIO_Init(GPIOA, &GPIO_InitStruct);			  // this passes the configuration to the Init function which takes care of the low level stuff
}

int main(void){
  
  // initialize the GPIO pins we need
  GPIO_init();

  /* This flashes the LEDs on the board once
   * Two registers are used to set the pins (pin level is VCC)
   * or to reset the pins (pin level is GND)
   */
  GPIOC->ODR = GPIO_Pin_8 | GPIO_Pin_9; // set PD8 and PC9

  while (1){  
		GPIOC->ODR |= GPIO_Pin_8; 		// set PC8 without affecting other bits
		
		if (GPIOA->IDR & GPIO_Pin_0)	// check if PA0 button is pressed
		{	
			GPIOC->ODR |= GPIO_Pin_9;	// turn on PC9
		}
		Delay(1000000L);				// wait a little
		
		GPIOC->ODR &= ~GPIO_Pin_9;		// clear PC8 and PC9
		Delay(1000000L);				// wait a little
	}
}
