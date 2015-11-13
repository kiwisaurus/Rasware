#include <stdio.h>
#include <RASLib/inc/common.h>
#include <RASLib/inc/gpio.h>
#include <RASLib/inc/time.h>
#include <RASLib/inc/motor.h>
#include <RASLib/inc/adc.h>
#include <RASLib/inc/linesensor.h>
#include <RASLib/inc/servo.h>

static tBoolean initialized[4] = {false,false,false,false};
static tMotor *Motors[2];
static tADC *Adc[2];
static tLineSensor *gls;
static tPWM *gate;
static tBoolean gatePos = true;

void initMotor(void){
    if(!initialized[0]){
        initialized[0] = true;


        
        //left wheel
        Motors[0] = InitializeServoMotor(PIN_A6, true);
        //right wheel
        Motors[1] = InitializeServoMotor(PIN_A5, false);
    }
}

void initADC(void){
     if(!initialized[1]){
         initialized[1] = true;
        
        //front ir
        Adc[0] = InitializeADC(PIN_D2);
        //left ir
        Adc[1] = InitializeADC(PIN_D1);
    }
}

void initGLine(void){
    if(!initialized[2]){
        initialized[2]=true;

        gls = InitializeGPIOLineSensor(
        PIN_F3,
        PIN_B3,
        PIN_C4,
        PIN_C5,
        PIN_C6,
        PIN_C7,
        PIN_D6,
        PIN_D7
        );
    }
}
void initServ(void){
    if(!initialized[3]){
        initialized[3] = true;
        gate = InitializeServo(PIN_E3);
    }
}
void moveGate(void){
    if(gatePos){
        SetServo(gate, 0.5);
        gatePos = false;
    }
    else{
        SetServo(gate,0);
        gatePos=true;
    }
}
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define SATURATE(min, x, max) ((x) < (min)) ? (min) : (((x) > (max)) ? (max) : (x))

void WallFollow(void){
    float speedL =1.0f;
    float speedR =1.0f;
    speedL = speedL + ADCRead(Adc[1])/3+ADCRead(Adc[0])/2;
    speedR = speedR - ADCRead(Adc[1])/3-ADCRead(Adc[0])/2;

    SetMotor(Motors[0] , SATURATE(0.0f, speedL, 1.0f));
    SetMotor(Motors[1], SATURATE(0.0f, speedR, 1.0f));
}

void LineFollow(float fLine[8]){
    float speedL = 1.0f;
    float speedR = 1.0f;

    for(int i=0; i<8; i++)
        if(fLine[i] >0.3f)
            fLine[i] = 1;

    speedL = speedL-fLine[7]*0.05f*8-fLine[6]*0.05f*4-fLine[5]*0.05f*2;
    speedR = speedR-fLine[0]*0.05f*8-fLine[1]*0.05f*4-fLine[2]*0.05f*2;

    SetMotor(Motors[0], SATURATE(0.0f, speedL, 1.0f));
    SetMotor(Motors[1], SATURATE(0.0f, speedR, 1.0f));
}
int main(void){
    initMotor();
    initADC();
    initGLine();
    LineSensorReadContinuously(gls, 0.2f);
    float fLine[8];
    tBoolean tLine[8];
    while(1){
        LineSensorReadArray(gls, fLine);
        
    }
}
