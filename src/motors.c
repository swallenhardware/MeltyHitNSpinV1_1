/*
 * motors.c
 *
 *  Created on: Mar 10, 2019
 *      Author: swallen
 */

#include "motors.h"

TIM_HandleTypeDef htim1;
TIM_TypeDef * timInst = TIM1;
DMA_TypeDef * dmaInst = DMA2;
DMA_Stream_TypeDef * dmaStrInst = DMA2_Stream1;

uint32_t lastMotorSendTime = 0;

int16_t motorSpeed = 0;
int16_t motorSetpoint = 0;
uint32_t lastSpeedChange = 0;


#ifdef MC_DSHOT
uint16_t motorPulseTrain[18];
#endif

void initMotors() {
	TIM_ClockConfigTypeDef sClockSourceConfig = {0};

	htim1.Instance = TIM1;
	__HAL_RCC_TIM1_CLK_ENABLE();
	__HAL_RCC_DMA2_CLK_ENABLE();

	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
	{
	Error_Handler();
	}

	GPIO_InitTypeDef GPIO_InitStruct = {0};

	__HAL_RCC_GPIOA_CLK_ENABLE();
	/**TIM1 GPIO Configuration
	PA10     ------> TIM1_CH3
	*/
	GPIO_InitStruct.Pin = GPIO_PIN_10;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.Alternate = GPIO_AF1_TIM1;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	HAL_TIM_PWM_Start(&htim1,TIM_CHANNEL_3);

#ifdef MC_ONESHOT125
	//setup the timer
	timInst->CR1 &= 0xFFFE; //disable timer

	timInst->CNT = 0; //clear counter

	timInst->CCMR2 |= 0x0060;//channel 3 is active when timer < count
	timInst->CCER |= 0x0100;//output compare 3 outputs are enabled
	timInst->ARR = ONESHOT_PERIOD;

	timInst->CR1 |= 0x0085;//enable timer, prescaler=1, auto-reload enabled
#endif

#ifdef MC_DSHOT
	//make sure the start and end are set to 0
	//The first packet is 0 because for some reason the first byte wasn't being transmitted in full
	//the last packet is 0 to make sure the line stays low when we aren't transmitting a code
	motorPulseTrain[0] = 0;
	motorPulseTrain[17] = 0;

	//setup the timer
	timInst->CR1 &= 0xFFFE; //disable timer

	//disable the dma stream
	dmaStrInst->CR &= 0xFFFFFFFE;
	//wait until the stream is disabled
	while(dmaStrInst->CR & 0x00000001);

	timInst->CNT = 0; //clear counter

	timInst->DIER |= 0x0200; //enable DMA requests on CH1 (no interrupts enabled)
	//timInst->DIER = 0x0100;//send dma request on update
	timInst->CCMR1 |= 0x0010;//channel 1 active when the timer matches
	timInst->CCMR2 |= 0x0060;//channel 3 is active when timer < count
	timInst->CCER |= 0x0101;//output compare 1 & 3 outputs are enabled

	timInst->ARR = DSHOT_PERIOD; //sets the frequency to the DSHOT baud rate

	//set the DMA trigger channels to fire right before the time rolls over
	timInst->CCR1 = DSHOT_PERIOD - 7;

	//setup the DMA on the peripheral side
	timInst->DCR |= 0x000F;//one transfer, starting from CCR3

	//setup the DMA peripheral
	dmaStrInst->CR = 0x0C032C40;//channel 6, very high priority, size 4 peripheral offset,
	//half word data sizes, memory increment, memory-to-peripheral

	dmaStrInst->M0AR = (uint32_t) motorPulseTrain;//set the peripheral address

	//enable the timer
	timInst->CR1 |= 0x0085; //enable timer, prescaler=1, auto-reload enabled
#endif
}

#ifdef MC_ONESHOT125
void disableMotors() {
	timInst->CR1 &= 0xFFFE; //disable timer
}

void enableMotors() {
	timInst->CR1 |= 0x0001;//enable timer
}
#endif

//helper function that accepts -MAX_SPEED to MAX_SPEED instead of a separate direction field
void setMotorSpeed(int16_t speed) {
	motorSetpoint = speed;
}

void runMotors() {
	uint32_t loopTime = getMicroseconds();

	//move the motor speed towards the setpoint
	if(loopTime - lastSpeedChange > MOTOR_ACCEL_RATE) {
		lastSpeedChange = loopTime;
		if(motorSpeed > motorSetpoint) motorSpeed--;
		if(motorSpeed < motorSetpoint) motorSpeed++;
	}

	uint16_t speed = (uint16_t) ((motorSpeed > 0) ? motorSpeed : motorSpeed*-1);

#ifdef MC_DSHOT
	//make sure we don't go *too* fast so the motor controllers don't freak out
	if(loopTime - lastMotorSendTime >= MIN_MOTOR_TIME) {
		lastMotorSendTime = loopTime;
#define DSHOT_RANGE 999U

		if(speed != 0) {//when speed == 0, we use command 0 instead of 1048. Command 0 arms and stops the motor
			//convert the speed value to be between 0 and 1000
			speed = ((uint32_t) speed * DSHOT_RANGE) / MOTOR_MAX_SPEED;
			if(speed > DSHOT_RANGE ) speed = DSHOT_RANGE;

			//adjust for the direction
			if(motorSpeed < 0) {
				speed = 1001 + speed;
			} else {
				//speed = DSHOT_RANGE - speed;
			}

			//add 48, as values 1-47 are reserved for special commands
			speed += 48;
		}

		issueDshotCommand(speed);
	}
#endif

#ifdef MC_ONESHOT125
	speed = ((uint32_t) speed * 2500U) / MOTOR_MAX_SPEED;

	//adjust for the direction
	if(motorSpeed < 0) {
		speed = 7500 + speed;
	} else {
		speed = 7500 - speed;
	}

	timInst->CCR3 = speed;
#endif
}

#ifdef MC_DSHOT
void armMotors() {
	HAL_Delay(500);
	issueDshotCommand(2047);
	HAL_Delay(500);
	issueDshotCommand(49);
}

void issueDshotCommand(uint16_t command) {

	//compute the checksum. xor the three nibbles of the speed + the telemetry bit (not used here)
	uint16_t checksum = 0;
	uint16_t checksum_data = command << 1;
	for(uint8_t i=0; i < 3; i++) {
		checksum ^= checksum_data;
		checksum_data >>= 4;
	}
	checksum &= 0x000F;//we only use the least-significant four bits as checksum
	uint16_t dshot_frame = (command << 5) | checksum; //add in the checksum bits to the least-four-significant bits

	//wait until the stream is disabled
	while(dmaStrInst->CR & 0x00000001);

	//Convert the bits to pulse lengths, then write the pulse lengths into the buffer
	for(int i=0; i<16; i++) {
		motorPulseTrain[i+1] = dshot_frame & 0x8000 ? DSHOT_1_TIME : DSHOT_0_TIME;
		dshot_frame <<= 1;
	}

	dmaStrInst->PAR = (uint32_t) &(timInst->CCR3);

	//start the dma transfer
	dmaInst->HIFCR = 0x0F7D0F7D;//clear all of the flags because I'm lazy
	dmaInst->LIFCR = 0x0F7D0F7D;
	dmaStrInst->NDTR = 18;//set transfer size to 18 bytes
	dmaStrInst->CR |= 0x00000001;//enable the dma stream

}
#endif
