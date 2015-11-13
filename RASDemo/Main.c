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


int main(void){
    initMotor();
    initADC();
    while(1){
       /* while(ADCRead(Adc[0]) > 3.0f){
            SetMotor(Motors[0], 1.0f);
            SetMotor(Motors[1], 0.0f);
        }
        if(ADCRead(Adc[1]) < 2.0f){
            while(ADCRead(Adc[1]) <2.0f){
            SetMotor(Motors[0], -1.0f);
            SetMotor(Motors[1], 1.0f);
            }
        }
        else if(ADCRead(Adc[1])>2.7f){
            while(ADCRead(Adc[1]) >2.8f){
            SetMotor(Motors[0], 1.0f);
            SetMotor(Motors[1], -1.0f);
            }
        }
        else{
            SetMotor(Motors[0], 1.0f);
            SetMotor(Motors[1], 1.0f);
        }*/
        if(ADCRead(Adc[0])> 0.5f){
            while(ADCRead(Adc[0]) >0.5){
            SetMotor(Motors[0], 1.0f);
            SetMotor(Motors[1], 0.0f);
            }
        }
        else if(ADCRead(Adc[1]) > 2.0f){
            while(ADCRead(Adc[1]) > 0.8f){
            SetMotor(Motors[0], 1.0f);
            SetMotor(Motors[1], 0.5f);
            }
        }
        else if(ADCRead(Adc[1]) <0.5f){
            while(ADCRead(Adc[1]) <0.5f){
            SetMotor(Motors[0], 0.5f);
            SetMotor(Motors[1], 1.0f);
            }
        }
        else{
            SetMotor(Motors[0], 1.0f);
            SetMotor(Motors[1], 1.0f);
        }
    }
}
