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
        Motors[0] = InitializeServoMotor(PIN_B6, true);
        //right wheel
        Motors[1] = InitializeServoMotor(PIN_B7, false);
    }
}

void initADC(void){
     if(!initialized[1]){
         initialized[1] = true;
        
        //front ir
        Adc[0] = InitializeADC(PIN_D0);
        //left ir
        Adc[1] = InitializeADC(PIN_D1);
    }
}


int main(void){
    initMotor();
    initADC();
    while(1){
        while(ADCRead(Adc[0]) > 2.4f){
            SetMotor(Motors[0], 1.0f);
            SetMotor(Motors[1], 0.0f);
        }
        if(ADCRead(Adc[1]) < 2.0f){
            SetMotor(Motors[0], 0.0f);
            SetMotor(Motors[1], 1.0f);
        }
        else if(ADCRead(Adc[1])>2.8f){
            SetMotor(Motors[0], 1.0f);
            SetMotor(Motors[1], 0.0f);
        }
        else{
            SetMotor(Motors[0], 1.0f);
            SetMotor(Motors[1], 1.0f);
        }
    }
}
