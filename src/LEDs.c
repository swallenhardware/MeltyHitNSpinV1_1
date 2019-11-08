/*
 * LEDs.c
 *
 *  Created on: Feb 23, 2019
 *      Author: swallen
 */

#include "LEDs.h"

TIM_TypeDef * ledTimInst = TIM3;

void initLEDs() {
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	//set up the gpio pins for tim3

	/**TIM3 GPIO Configuration
	PC6     ------> TIM1_CH1
	PC7     ------> TIM1_CH2
	PB0     ------> TIM1_CH3
	*/
	GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = GPIO_PIN_0;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	//set up the timer
	__HAL_RCC_TIM3_CLK_ENABLE();

	//disable the timer
	ledTimInst->CR1 = 0;

	//set up the prescaler to give the correct frequency
	//this needs to be higher than the "fusion frequency" for POV displays, low enough that the BJT's/LEDs can handle
	ledTimInst->PSC = (MAIN_CLOCK/2)-1;//gives 2MHz. Given a value of 100 in ARR, that means a 20KHz cycle

	//channels 1/2/3 are active when TIM_CNT < CCRx.
	ledTimInst->CCMR1 = 0x6060;
	ledTimInst->CCMR2 = 0x0060;

	//enable capture/compare outputs
	ledTimInst->CCER = 0x1111;

	//make sure thee CCR registers are zeroed. These will set the duty cycle of the color channel later
	ledTimInst->CCR1 = 0;
	ledTimInst->CCR2 = 0;
	ledTimInst->CCR3 = 0;

	//we reset the timer at 100. 100 is used to make the value more understandable, it's a percent!
	ledTimInst->ARR = 100;

	//enable the timer
	ledTimInst->CR1 = 0x0001;
}

void setLEDs(uint8_t r, uint8_t g, uint8_t b) {
	ledTimInst->CCR1 = r;
	ledTimInst->CCR2 = g;
	ledTimInst->CCR3 = b;
}
