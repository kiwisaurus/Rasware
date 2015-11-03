#include <RASLib/inc/common.h>
#include <RASLin/inc/motor.h>

static tMotor *Motors[2];
static tBoolean initialized = false;

void InitMotors(void){
	if(!initialized){
		initialized = true;

		Motors[0] = InitializeServoMotor(PIN_A5, false);
		Motors[1] = InitializeServoMotor(PIN_A6, true);
	}
}

void main(void){
	InitMotors();
	InitializeSystemTime(void);
	SetMotor(Motors[0], 0.5f);
	SetMotor(Motors[1], 0.5f);
	while(GetTime()<10.0f){
	}
	SetMotor(Motors[0], 0.0f);
	SetMotor(Motors[1], 0.0f);
}
