/*
 * LEDs.h
 *
 *  Created on: Feb 24, 2019
 *      Author: swallen
 */

#ifndef LEDS_H_
#define LEDS_H_

#include <stdlib.h>
#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"
#include "params.h"

extern void initLEDs(void);
extern void setLEDs(uint8_t r, uint8_t g, uint8_t b);

#endif /* LEDS_H_ */
