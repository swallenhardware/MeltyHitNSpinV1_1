/*
 * motors.h
 *
 *  Created on: Mar 10, 2019
 *      Author: swallen
 */

#ifndef MOTORS_H_
#define MOTORS_H_

#include "stm32f4xx_hal.h"
#include "params.h"
#include "time.h"

#define MC_DSHOT
//#define MC_ONESHOT125

#define MOTOR1 1
#define MOTOR2 2

#define MOTOR_MAX_SPEED 512

#define MOTOR_ACCEL_RATE 8//in us per LSB. gives a 4ms ramp time
//#define MOTOR_ACCEL_RATE 3906//gives a 2s ramp time, for testing the ramp code

#define MOTOR_DEADZONE 100//the minimum speed value before the motors are engaged

void initMotors(void);

void setMotorSpeed(int16_t speed);

void runMotors(void);

#ifdef MC_DSHOT
	//#define DSHOT300
	//#define DSHOT600
	#define DSHOT1200

//the minimum amount of time before a new dshot command should be sent, in us
	#ifdef DSHOT300
		#define MIN_MOTOR_TIME 400UL
	#endif

	#ifdef DSHOT600
		#define MIN_MOTOR_TIME 200UL
	#endif

	#ifdef DSHOT1200
		#define MIN_MOTOR_TIME 100UL
	#endif

//these values represent the correct tick counts to create various DSHOT baud rates at various base clock frequencies
#if MAIN_CLOCK == 60//in MHz
	#ifdef DSHOT300
		#define DSHOT_PERIOD 100
		#define DSHOT_1_TIME 75
		#define DSHOT_0_TIME 37
	#endif

	#ifdef DSHOT600
		#define DSHOT_PERIOD 50
		#define DSHOT_1_TIME 38 //75% of 600 baud, tuned for 1250ns
		#define DSHOT_0_TIME 19 //37% of 600 baud, tuned for 625ns
	#endif

	#ifdef DSHOT1200
		#define DSHOT_PERIOD 25
		#define DSHOT_1_TIME 19
		#define DSHOT_0_TIME 9
	#endif
#endif

#if MAIN_CLOCK == 80
	#ifdef DSHOT300
		#define DSHOT_PERIOD 132
		#define DSHOT_1_TIME 104
		#define DSHOT_0_TIME 52
	#endif

	#ifdef DSHOT600
		#define DSHOT_PERIOD 66
		#define DSHOT_1_TIME 52
		#define DSHOT_0_TIME 26
	#endif

	#ifdef DSHOT1200
		#define DSHOT_PERIOD 33
		#define DSHOT_1_TIME 26
		#define DSHOT_0_TIME 13
	#endif
#endif

#if MAIN_CLOCK == 120
	#ifdef DSHOT300
		#define DSHOT_PERIOD 200
		#define DSHOT_1_TIME 150
		#define DSHOT_0_TIME 74
	#endif

	#ifdef DSHOT600
		#define DSHOT_PERIOD 100
		#define DSHOT_1_TIME 75
		#define DSHOT_0_TIME 37
	#endif

	#ifdef DSHOT1200
		#define DSHOT_PERIOD 50
		#define DSHOT_1_TIME 38
		#define DSHOT_0_TIME 19
	#endif
#endif

//this typedef taken from src\main\drivers\pwm_output.h in the betaflight github page
typedef enum {
    DSHOT_CMD_MOTOR_STOP = 0,
    DSHOT_CMD_BEACON1,
    DSHOT_CMD_BEACON2,
    DSHOT_CMD_BEACON3,
    DSHOT_CMD_BEACON4,
    DSHOT_CMD_BEACON5,
    DSHOT_CMD_ESC_INFO, // V2 includes settings
    DSHOT_CMD_SPIN_DIRECTION_1,
    DSHOT_CMD_SPIN_DIRECTION_2,
    DSHOT_CMD_3D_MODE_OFF,
    DSHOT_CMD_3D_MODE_ON,
    DSHOT_CMD_SETTINGS_REQUEST, // Currently not implemented
    DSHOT_CMD_SAVE_SETTINGS,
    DSHOT_CMD_SPIN_DIRECTION_NORMAL = 20,
    DSHOT_CMD_SPIN_DIRECTION_REVERSED = 21,
    DSHOT_CMD_LED0_ON, // BLHeli32 only
    DSHOT_CMD_LED1_ON, // BLHeli32 only
    DSHOT_CMD_LED2_ON, // BLHeli32 only
    DSHOT_CMD_LED3_ON, // BLHeli32 only
    DSHOT_CMD_LED0_OFF, // BLHeli32 only
    DSHOT_CMD_LED1_OFF, // BLHeli32 only
    DSHOT_CMD_LED2_OFF, // BLHeli32 only
    DSHOT_CMD_LED3_OFF, // BLHeli32 only
    DSHOT_CMD_AUDIO_STREAM_MODE_ON_OFF = 30, // KISS audio Stream mode on/Off
    DSHOT_CMD_SILENT_MODE_ON_OFF = 31, // KISS silent Mode on/Off
    DSHOT_CMD_SIGNAL_LINE_TELEMETRY_DISABLE = 32,
    DSHOT_CMD_SIGNAL_LINE_CONTINUOUS_ERPM_TELEMETRY = 33,
    DSHOT_CMD_MAX = 47
} dshotCommands_e;

void issueDshotCommand(uint16_t command);
void armMotors(void);
#endif

#ifdef MC_ONESHOT125
#define ONESHOT_PERIOD 10000
void disableMotors(void);
void enableMotors(void);
#endif


#endif /* MOTORS_H_ */
