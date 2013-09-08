#include <stm32f10x.h>

void Delay(__IO uint32_t nCount) {
  while(nCount--) {
  }
}

/* This funcion shows how to initialize 
 * the GPIO pins on GPIOD and how to configure
 * them as inputs and outputs 
 */
void init_GPIO(void){
	
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
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
	
	/* In this block of instructions all the properties
	 * of the peripheral, the GPIO port in this case,
	 * are filled with actual information and then 
	 * given to the Init function which takes care of 
	 * the low level stuff (setting the correct bits in the 
	 * peripheral's control register)
	 * PD12 thru PD15
	 */
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_15 | GPIO_Pin_14 | GPIO_Pin_13 | GPIO_Pin_12; // we want to configure all LED GPIO pins
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP; 	// we want the pins to be a push-pull output
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz; 	// this sets the GPIO modules clock speed
	GPIO_Init(GPIOD, &GPIO_InitStruct); 			// this finally passes all the values to the GPIO_Init function which takes care of setting the corresponding bits.
		
	/* Here the GPIOD module is initialized.
	 * We want to use PD0 as an input because
	 * the USER button on the board is connected
	 * between this pin and VCC.
	 */
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;		  // we want to configure PA0
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPD; 	  // we want it to be an input with a pull-down resistor enabled
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;//this sets the GPIO modules clock speed
	GPIO_Init(GPIOA, &GPIO_InitStruct);			  // this passes the configuration to the Init function which takes care of the low level stuff
}

int main(void){
  
  // initialize the GPIO pins we need
  init_GPIO();

  /* This flashed the LEDs on the board once
   * Two registers are used to set the pins (pin level is VCC)
   * or to reset the pins (pin level is GND)
   * 
   * BSRR stands for bit set/reset register
   * it is seperated into a high and a low word (each of 16 bit size)
   * 
   * A logical 1 in BSRR will set the pin and a logical 1 in BRR will
   * reset the pin. A logical 0 in either register has no effect
   */
  GPIOD->BSRR = 0xF000; // set PD12 thru PD15
  Delay(1000000L);		 // wait a short period of time
  GPIOD->BRR = 0xF000; // reset PD12 thru PD15
  
  // this counter is used to count the number of button presses
  uint8_t i = 0; 

  while (1){  
    
		/* Every GPIO port has an input and 
		 * output data register, ODR and IDR 
		 * respectively, which hold the status of the pin
		 * 
		 * Here the IDR of GPIOD is checked whether bit 0 is
		 * set or not. If it's set the button is pressed
		 */
		if(GPIOD->IDR & 0x0001){
			// if the number of button presses is greater than 4, reset the counter (we start counting from 0!)
			if(i > 3){
				i = 0;
			}
			else{ // if it's smaller than 4, switch the LEDs
			
				switch(i){
				
					case 0:
						GPIOD->BSRR = 0x1000; // this sets LED1 (green)
						GPIOD->BRR = 0x8000; // this resets LED4 (blue)
						break;
				
					case 1:
						GPIOD->BSRR = 0x2000; // this sets LED2 (orange)
						GPIOD->BRR = 0x1000; // this resets LED1 
						break;
							
					case 2:
						GPIOD->BSRR = 0x4000; // this sets LED3 (red)
						GPIOD->BRR = 0x2000; // this resets LED2
						break;
							
					case 3: 
						GPIOD->BSRR = 0x8000; // this sets LED4
						GPIOD->BRR = 0x4000; // this resets LED3
						break;
					}
				
				i++; // increase the counter every time the switch is pressed
			}
			Delay(3000000L); // add a small delay to debounce the switch
		}
	}
}
