#include <stm32f10x.h>

void Delay(__IO uint32_t nCount) {
  while(nCount--) {
  }
}

#define TIM3_CCR1_Address 0x40000434	// physical memory address of Timer 3 CCR1 register

TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
TIM_OCInitTypeDef  TIM_OCInitStructure;
GPIO_InitTypeDef GPIO_InitStructure;
DMA_InitTypeDef DMA_InitStructure;

uint16_t LED_BYTE_Buffer[42];	// buffer that holds one complete transmission

// this buffer holds a 12 step color wheel represented
// by individual R, G and B values
// no gamma correction has been applied
uint8_t colors[12][3] = 
{
	{255, 0, 0},
	{255, 127, 0},
	{255, 255, 0},
	{127, 255, 0},
	{0, 255, 0},
	{0, 255, 127},
	{0, 255, 255},
	{0, 127, 255},
	{0, 0, 255},
	{127, 0, 255},
	{255, 0, 255},
	{255, 0, 127},
};

void Timer3_init(void)
{
	uint16_t PrescalerValue;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	/* GPIOA Configuration: TIM3 Channel 1 as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	/* Compute the prescaler value */
	PrescalerValue = (uint16_t) (SystemCoreClock / 24000000) - 1;
	/* Time base configuration */
	TIM_TimeBaseStructure.TIM_Period = 29; // 800kHz 
	TIM_TimeBaseStructure.TIM_Prescaler = PrescalerValue;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

	/* PWM1 Mode configuration: Channel1 */
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = 0;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OC1Init(TIM3, &TIM_OCInitStructure);
	
	/* configure DMA */
	/* DMA clock enable */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	
	/* DMA1 Channel6 Config */
	DMA_DeInit(DMA1_Channel6);

	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)TIM3_CCR1_Address;	// physical address of Timer 3 CCR1
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)LED_BYTE_Buffer;		// this is the buffer memory 
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;						// data shifted from memory to peripheral
	DMA_InitStructure.DMA_BufferSize = 42;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;					// automatically increase buffer index
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;							// stop DMA feed after buffer size is reached
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	
	DMA_Init(DMA1_Channel6, &DMA_InitStructure);
	
	/* TIM3 CC1 DMA Request enable */
	TIM_DMACmd(TIM3, TIM_DMA_CC1, ENABLE);
}

void WS2812_send(uint8_t red, uint8_t green, uint8_t blue)
{
	uint8_t color[3];	// holds the color values in the correct order
	uint8_t i,j;		// index variables to access single bit in array
	uint8_t mem;		// index variable to access byte in transmit buffer
	
	// bring the color bytes into correct order
	color[0] = green;	
	color[1] = red;
	color[2] = blue;

	// fill transmit buffer with correct compare values to achieve
	// correct pulse widths according to color values
	mem = 0;	// reset buffer memory index
	
	for (i = 0; i < 3; i++)
	{
		for (j = 0; j < 8; j++)
		{
			if ( (color[i]<<j) & 0x80 )		// data sent MSB first, j = 0 is MSB j = 7 is LSB
			{
				LED_BYTE_Buffer[mem] = 17; 	// compare value for logical 1
			}
			else
			{
				LED_BYTE_Buffer[mem] = 9;	// compare value for logical 0
			}
			mem++;
		}
	}
	
	// add needed delay at end of byte cycle, pulsewidth = 0
	while(mem < 42)
	{
		LED_BYTE_Buffer[mem] = 0;
		mem++;
	}

	DMA_SetCurrDataCounter(DMA1_Channel6, 42); 	// load number of bytes to be transferred
	DMA_Cmd(DMA1_Channel6, ENABLE); 			// enable DMA channel 6
	TIM_Cmd(TIM3, ENABLE); 						// enable Timer 3
	while(!DMA_GetFlagStatus(DMA1_FLAG_TC6)); 	// wait until transfer complete
	TIM_Cmd(TIM3, DISABLE); 					// disable Timer 3
	DMA_Cmd(DMA1_Channel6, DISABLE); 			// disable DMA channel 6
	DMA_ClearFlag(DMA1_FLAG_TC6); 				// clear DMA1 Channel 6 transfer complete flag
}

int main(void) {
	Timer3_init();
	
	int16_t i;
	
	while (1){  
		for (i = 0; i < 12; i++)
		{
			WS2812_send(colors[i][0], colors[i][1], colors[i][2]);
			Delay(500000L);
		}
	}
}
