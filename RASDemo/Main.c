/*#include "RASDemo.h"

#include <RASLib/inc/common.h>
#include <RASLib/inc/gpio.h>
#include <RASLib/inc/time.h>

tBoolean led_on;

void blink(void) {
    SetPin(PIN_F1, led_on);
    SetPin(PIN_F3, !led_on);

    led_on = !led_on;
}

int main(void) {  
    char ch;

    CallEvery(blink, 0, 0.25f);

    while (1) {
        Printf("\nRAS Demo for Robotathon 2013\n"
               "  0=UART Demo\n"
	       "  1=Motor Demo\n"
               "  2=Servo Demo\n"
	       "  3=I2C Line Sensor Demo\n"
               "  4=IR Sensor Demo\n"
	       "  5=Encoders Demo\n"
               "  6=GPIO Demo\n"
	       "  7=GPIO Line Sensor Demo\n"
               "  8=Sonar Demo\n"
               ">> ");
        
        // Read input from user
        ch = Getc();
        Printf("\n");

        switch(ch) {
            case '0':
                Printf("\nUART Demo\n");
                uartDemo();                 
                break;
            case '1':
                Printf("\nMotor Demo\n");
                initMotors();
                motorDemo(); 
                break;
            case '2':
                Printf("\nServo Demo\n");
                initServo();
                servoDemo();     
                break;
            case '3':
                Printf("\nLine Sensor Demo\n");
                initI2CLineSensor();          
                i2cLineSensorDemo();    
                break;
            case '4':                            
                Printf("\nIR Sensor Demo\n");
                initIRSensor();
                IRSensorDemo();     
                break;
            case '5':
                Printf("\nEncoders Demo\n");
                initEncoders();
                encoderDemo();
                break;
            case '6':
                Printf("\nGPIO Demo\n");
                gpioDemo();
                break;
            case '7':
                Printf("\nGPIO Line Sensor Demo\n");
                initGPIOLineSensor();
                gpioLineSensorDemo();
                break;
            case '8':
                Printf("\nSonar Demo\n");
                initSonar();
                sonarDemo();
                break;
        }
    }
}
*/
#include <RASLib/inc/common.h>
#include <RASLib/inc/motor.h>
#include <RASLib/inc/adc.h>
#include <RASLib/inc/time.h>

static tMotor *Motors[2];
static tADC *adc[2];
//[0] wheels
//[1] IR Sensor
static tBoolean initialized[2] = {false,false};


void InitMotors(void){
	if(!initialized[0]){
		initialized[0] = true;

		//right
		Motors[0] = InitializeServoMotor(PIN_A5, true);
		//left
		Motors[1] = InitializeServoMotor(PIN_A6, false);
	}
}

void InitIRSensors(void){
	if(!initialized[1]){
		initialized[1] = true;
		
		//front
		adc[0] = InitializeADC(PIN_D0);
		//left
		adc[1] = InitializeADC(PIN_D1);
	}
}

void main(void){
	InitMotors();
	InitializeSystemTime();
	while(1){
		if((ADCRead(adc[0]))>2.0f){
			setMotor(Motors[0], 0.7f);
			setMotor(Motors[1], 0.2f);
			while(ADCRead(adc[0]>2.0f)){}
		}
		else if((ADCRead(adc[1]))>2.9f){
			setMotor(Motors[0], 0.4f);
			setMotor(Motors[1], 0.6f);
		}
		else if((ADCRead(adc[1])<1.0f)){
			setMotor(Motors[0], 0.6f);
			setMotor(Motors[1], 0.4f);
		}
		else{
			setMotor(Motors[0], 0.5f);
			setMotor(Motors[1], 0.5f);
		}
	}
}
