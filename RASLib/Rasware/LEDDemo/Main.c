#include "RasDemo.h"

#include <RasLib/inc/common.h>
#include <RasLib/inc/gpio.h>
#include <RASLib/inc/time.h>

tBoolean led_on;

void blink(void){
	SetPin(PIN_F1, led_on);
	led_on=!led_on;
}

int main(void){
	callEvery(blink, 0, 0.25f);
}
