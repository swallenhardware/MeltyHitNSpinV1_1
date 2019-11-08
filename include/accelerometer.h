/*
 * accelerometer.h
 *
 *  Created on: Mar 10, 2019
 *      Author: swallen
 */

#ifndef ACCELEROMETER_H_
#define ACCELEROMETER_H_

#include "stm32f4xx_hal.h"
#include "math.h"
#include "time.h"
#include "serial.h"
#include "adcs.h"

#define ACCEL_I2C_ADDRESS (0x18<<1)

#define ACCEL_WHO_AM_I 0x0F
#define ACCEL_CTRL_REG1 0x20
#define ACCEL_CTRL_REG2 0x21
#define ACCEL_CTRL_REG3 0x22
#define ACCEL_CTRL_REG4 0x23
#define ACCEL_CTRL_REG5 0x24
#define ACCEL_HP_FILTER_RESET 0x25
#define ACCEL_REFERENCE 0x26
#define ACCEL_STATUS_REG 0x27
#define ACCEL_OUT_X_L 0x28
#define ACCEL_OUT_X_H 0x29
#define ACCEL_OUT_Y_L 0x2A
#define ACCEL_OUT_Y_H 0x2B
#define ACCEL_OUT_Z_L 0x2C
#define ACCEL_OUT_Z_H 0x2D
#define ACCEL_INT1_CFG 0x30
#define ACCEL_INT1_SRC 0x31
#define ACCEL_INT1_THS 0x32
#define ACCEL_INT1_DURATION 0x33
#define ACCEL_INT2_CFG 0x34
#define ACCEL_INT2_SRC 0x35
#define ACCEL_INT2_THS 0x36
#define ACCEL_INT2_DURATION 0x37

#define MAG_I2C_ADDRESS 0x38

#define MAG_WHO_AM_I 0x0F
#define MAG_CTRL_REG1 0x20
#define MAG_CTRL_REG2 0x21
#define MAG_CTRL_REG3 0x22
#define MAG_CTRL_REG4 0x23
#define MAG_CTRL_REEG5 0x24
#define MAG_STATUS_REG 0x27
#define MAG_OUT_X_L 0x28
#define MAG_OUT_X_H 0x29
#define MAG_OUT_Y_L 0x2A
#define MAG_OUT_Y_H 0x2B
#define MAG_OUT_Z_L 0x2C
#define MAG_OUT_Z_H 0x2D
#define MAG_TEMP_OUT_L 0x2E
#define MAG_TEMP_OUT_H 0x2F
#define MAG_INT_CFG 0x30
#define MAG_INT_SRC 0x31
#define MAG_INT_THS_L 0x32
#define MAG_INT_THS_H 0x33

void initAccelerometer(void);

void runMagnetometer(void);
int16_t getMagX(void);
int16_t getMagY(void);
int16_t getMagZ(void);

void runAccelerometer(void);
int16_t getAccelAngle(void);

void runHybridSensing(void);
int16_t getHybridAngle(void);

_Bool upToSpeed(void);

uint8_t writeI2CReg8Blocking(I2C_HandleTypeDef *i2c, uint8_t addr, uint8_t subaddr, uint8_t data, uint32_t timeout);
uint8_t readI2CReg16Blocking(I2C_HandleTypeDef *i2c, uint8_t addr, uint8_t subaddr, uint16_t *data, uint32_t timeout);

void I2C_ClearBusyFlagErratum(void);

#endif /* ACCELEROMETER_H_ */
