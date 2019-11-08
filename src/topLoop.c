/*
 * topLoop.c
 *
 * this contains the main state machine logic for the bot
 *
 *  Created on: Mar 8, 2019
 *      Author: swallen
 */

#include "topLoop.h"

uint8_t mainState = 0;//we set to an invalid state so the loop is forced to properly set up in safe state

uint32_t loopTime = 0;//in us

uint8_t stickSwitchDebounce = 0;

int16_t beaconOffset = 90;

uint8_t led_on = 0;
uint32_t led_time = 0;

void loop() {

	//USER INITIALIZATION FUNCTIONS
	initTimers();
	initLEDs();
	initADCs();
	initSerial();
	initWatchdog();
	initMotors();
	initAccelerometer();

	//turnSourceOn();

	//INFINITE LOOP
	while(1) {
		feedWatchdog();
		loopTime = getMicroseconds();

		receiveSerial();

		//make sure we are still connected to the controller
		if(!isControlled() && mainState != STATE_SAFE){
			setState(STATE_SAFE);
		}

		//switch debouncing functions
		if(stickSwitchDebounce && !getStickSwitch()) {
			//switch released
			stickSwitchDebounce = 0;
		} else if(!stickSwitchDebounce && getStickSwitch()) {
			//switch pressed
			stickSwitchDebounce = 1;
			beaconOffset = (beaconOffset == 90) ? 270 : 90;
		}

		switch(mainState) {
		case(STATE_SAFE):
			//This state is for fault conditions, and for when we don't have communication with the controller.
			if(isControlled()) setState(STATE_IDLE);
			break;

		case(STATE_IDLE):
			//this state is when we have communications with the controller, but it's telling us to stay disabled
			if(isEnabled()) setState(STATE_SPIN);

			setMotorSpeed(0);//make damn sure the motor is stopped
			runMotors();

			if((getMilliseconds() - led_time) > 1000) {
				if(led_on) {
					setLEDs(50, 50, 0);
					led_on = 0;
				} else if(!led_on) {
					setLEDs(100, 100, 0);
					led_on = 1;
				}
				led_time = getMilliseconds();
			}
			break;

		case(STATE_SPIN):
			//this state drives the robot in meltybrain mode
			if(!isEnabled()) setState(STATE_IDLE);


			//this is the master heading calculation. If we want to change what sensor combinations are feeding us
			//our heading, we do that in the following lines
   			//runBeacon();
			//int16_t heading = getBeaconAngle();
   			//runAccelerometer();
			//int16_t heading = getAccelAngle();
			runHybridSensing();
			int16_t heading = (getHybridAngle() + beaconOffset) % 360;

			int16_t speed = ((int16_t) (commandedThrottle())/2)*getDirSwitch();

			//first check if the melty throttle is high enough for translation
			if (getMeltyThrottle() > 10) {

				//calculate the distance between the current heading and the commanded direction
				int16_t diff = 180 - abs(abs(getMeltyAngle() - heading) - 180);

				//now check if we are pointed more towards the commanded direction or the opposite
				if(diff < 90) {
				  //we are pointing towards the commanded heading, forward pulse
				  setMotorSpeed(speed/2);
				} else {
				  //we are pointing opposite the commanded heading, reverse pulse
				  setMotorSpeed(speed);
				}

				//ANIMATION
				if(diff < 10) {
					setLEDs(0, 0, 100);
				} else if(heading < 180) {
					setLEDs(0, 100, 0);
				} else {
					setLEDs(100, 0, 0);
				}
			} else {
				//if we aren't translating, just run the motors at the throttle speed
				setMotorSpeed(speed);

				//ANIMATION
				if(heading < 180) {
					setLEDs(0, 100, 0);
				} else {
					setLEDs(100, 0, 0);
				}
			}

			runMotors();

			break;

		default:
			setState(STATE_SAFE);
			break;
		}
	}
}

/* setState()
 * arguments:
 * 	newState: the new state that you wish to transition the bot into
 * returns:
 *  nothing
 *
 *  Use this to change what state the bot is in. This function handles all relevant re-initialization
 *  for every state.
 */

void setState(uint8_t newState) {
	switch(newState) {
	default:


	case(STATE_SAFE):
		mainState = newState;

    	setLEDs(100, 0, 0);

#ifdef MC_ONESHOT125
		disableMotors();
#endif
		break;


	case(STATE_IDLE):
		mainState = newState;

#ifdef MC_ONESHOT125
		setMotorSpeed2(MOTOR1, 0);
#endif
		break;


	case(STATE_SPIN):
		mainState = newState;

#ifdef MC_ONESHOT125
		enableMotors();
#endif
		break;


	}
}
