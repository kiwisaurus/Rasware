#include <stdio.h>
#include <RASLib/inc/common.h>
#include <RASLib/inc/gpio.h>
#include <RASLib/inc/time.h>
#include <RASLib/inc/motor.h>
#include <RASLib/inc/adc.h>

static tBoolean initialized[2] = {false,false};
static tMotor *Motors[2];
static tADC *Adc[2];

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
float SpeedFix(float speed){
    if(speed<0.0f)
        return 0.0f;
    else if(speed>1.0f)
        return 1.0f;
    else
        return speed;
}

int main(void){
    initMotor();
    initADC();
    while(1){
        float speedL=0.3f+ADCRead(Adc[1])/2-ADCRead(Adc[0])/3;
        float speedR=0.3f-ADCRead(Adc[1])/2+ADCRead(Adc[0])/3;
        
        speedL= SpeedFix(speedL);
        speedR= SpeedFix(speedR);

        SetMotor(Motors[0], speedL);
        SetMotor(Motors[1], speedR);
    }
}
