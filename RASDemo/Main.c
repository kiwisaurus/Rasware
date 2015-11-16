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
static tBoolean gatePos = false;

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
void LineThreshold(float *fLine){
    float threshold = 0.4f;//change this
    for(int i =0; i<8; i++){
        if(fLine[i]>=threshold)
            fLine[i] =1; //pseudo boolean
}

void LineFollow(float *fLine){
    float speedL = 1.0f;
    float speedR = 1.0f;

    speedL = speedL-fLine[7]*0.05f*8-fLine[6]*0.05f*4-fLine[5]*0.05f*2;
    speedR = speedR-fLine[0]*0.05f*8-fLine[1]*0.05f*4-fLine[2]*0.05f*2;

    SetMotor(Motors[0], SATURATE(0.0f, speedL, 1.0f));
    SetMotor(Motors[1], SATURATE(0.0f, speedR, 1.0f));
}
tBoolean IntersectCheck(float *fLine){
    int sum =0;
    for(int i =0;i<8;i++)
        sum = sum+fLine[i];
    if(sum>3) //assumes the array has been through lineThreshold()
        return true;
    return false;
}

void kill(void){
    SetMotor(Motors[0], 0.0f);
    SetMotor(Motors[1], 0.0f);
    while(1){}
}

int main(void){
    CallIn(kill, 0, 120);
    InitializeSystemTime(); //not accurate for long times
    initMotor();
    initADC();
    initGLine();
    LineSensorReadContinuously(gls, 0.2f);
    float *fLine;
    int stage =1;
    tBoolean tLine[8];
    int intersects =0;
    moveGate(); //open gate
    
    char *state[8] = {       //state machine
        "START", "LINE_2/5", 
        "SCORE_1/4","SCORE_3/6",
        "WALL_6/5/4","WALL_1/2/3",
        "SCORE_3/6R", "RETURN"
    };
    int statePos = 0; //used to traverse states


    while(1){
        switch(statePos){

        case 0:
            LineSensorReadArray(gls, fLine);
            LineThreshold(fLine); //call everytime we read array
            if(IntersectCheck(fLine)){
                statePos = 1; //advance state
                intersects++;
                float time1 = GetTime();
                SetMotor(Motors[0], 0.7f);
                SetMotor(Motors[1], 1.0f); //take the line on the left
                while(GetTime()-time1<0.5){} //run for half a second
            }
            else
                LineFollow(fLine);
            break;

        case 1:
            LineSensorReadArray(gls, fLine);
            LineThreshold(fLine);
            if(IntersectCheck(fLine)){
                intersects++;
                if(intersects%2 ==1){ //one loop around the middle
                    statePos = 3; //SCORE_3/6
                    float time1 = GetTime();
                    SetMotor(Motors[0], 1.0f);
                    SetMotor(Motors[1], 0.0f); //turn to face middle
                    while(GetTime()-time1>0.5f){}//half a second
                    time1 = GetTime(); //update reference point
                    SetMotor(Motors[0], 1.0f);
                    SetMotor(Motors[0], 1.0f);
                    while(GetTime()-time1<0.3f){} //get off the intersection
                }
            }
            else
                LineFollow(fLine);
            break;

            case 3: //score ping pong
                LineSensorReadArray(gls, fLine);
                LineThreshold(fLine); //why don't I put these 2 together?
                if(ADCRead(Adc[0])>0.3f){ //approaching wall
                    SetMotor(Motors[0], 1.0f);
                    SetMotor(Motors[1], 1.0f);
                    while(ADCRead(Adc[0])<0.5f){} //not too close
                    moveGate(); //hold in marbles
                    float speed = 1.0f;
                    while(speed>0.0f){
                        SetMotor(Motors[0], SATURATE(0.0f,speed,1.0f));
                        SetMotor(Motors[1], SATURATE(0.0f,speed,1.0f));
                        speed = speed-0.2f;
                    } //pseudo deccelearte to score
                    statePos = 4; //WALL_6/5/4
                    float time1 = GetTime();
                    SetMotor(Motors[0], -0.3f);
                    SetMotor(Motors[1], -0.3f);
                    while(GetTime()-time1 <0.3f){} //backup out of the wall
                    moveGate(); //grab more marbles
                    time1 = GetTime(); //update reference point
                    SetMotor(Motors[0],1.0f);
                    SetMotor(Motors[1], 0.5f);
                    while(GetTime()-time1<0.4f){} //turn right
                }
                else
                    LineFollow(fLine);
                break;

            case 4:
                LineSensorReadArray(gls, fLine);
                LineThreshold(fLine); //why do I not just run this before the switch

                if(IntersectCheck(fLine)){ //reached goal on other side
                    moveGate(); //hold in marbles while adjusting
                    float time1 = GetTime();
                    SetMotor(Motors[0], -1.0f);
                    SetMotor(Motors[1],-1.0f);
                    while(GetTime()-time1<0.3f){}//backup
                    time1 = GetTime();
                    SetMotor(Motors[0], 1.0f);
                    SetMotor(Motors[1], 0.4f);
                    while(!IntersectCheck(fLine)){}//foward and right to line
                    time1 =GetTime();
                    SetMotor(Motors[0], -1.0f);
                    SetMotor(Motors[1], 1.0f);
                    while(GetTime()-time1<0.4f){} //turn to face wall
                    statePos = 2; //SCORE_1/4
                }
                else
                    WallFollow();
                break;

            case 2: //this is in the order it happens, not the ordered it's stored
                LineSensorReadArray(gls, fLine);
                LineThreshold(fLine); //why don't I put these 2 together?
                if(ADCRead(Adc[0])>0.3f){ //approaching wall
                    SetMotor(Motors[0], 1.0f);
                    SetMotor(Motors[1], 1.0f);
                    while(ADCRead(Adc[0])<0.5f){} //not too close
                    moveGate(); //open gate
                    float speed = 1.0f;
                    while(speed>0.0f){
                        SetMotor(Motors[0], SATURATE(0.0f,speed,1.0f));
                        SetMotor(Motors[1], SATURATE(0.0f,speed,1.0f));
                        speed = speed-0.2f;
                    } //pseudo deccelearte to score
                    statePos = 5; //WALL_1/2/3
                    float time1 = GetTime();
                    SetMotor(Motors[0], -0.3f);
                    SetMotor(Motors[1], -0.3f);
                    while(GetTime()-time1 <0.3f){} //backup out of the wall
                    time1 = GetTime(); //update reference point
                    SetMotor(Motors[0],1.0f);
                    SetMotor(Motors[1], 0.5f);
                    while(GetTime()-time1<0.4f){} //turn right
                }
                else
                    LineFollow(fLine);
                break;
                
                case 5:
                    LineSensorReadArray(gls, fLine);
                LineThreshold(fLine); //why do I not just run this before the switch

                if(IntersectCheck(fLine)){ //reached goal on other side
                    moveGate(); //hold in marbles while adjusting
                    float time1 = GetTime();
                    SetMotor(Motors[0], -1.0f);
                    SetMotor(Motors[1],-1.0f);
                    while(GetTime()-time1<0.3f){}//backup
                    time1 = GetTime();
                    SetTime(Motors[0], 1.0f);
                    SetMotor(Motors[1], 0.4f);
                    while(!IntersectCheck(fLine)){}//foward and right to line
                    time1 =GetTime();
                    SetMotors(Motors[0], -1.0f);
                    SetMotors(Motors[1], 1.0f);
                    while(GetTime()-time1<0.4f){} //turn to face wall
                    statePos = 6; //SCORE_3/6R
                }
                else
                    WallFollow();
                break;
            
            case 6: //score in 3/6 then return to starting position
            //assume gate is closed
                LineSensorReadArray(gls, fLine);
                LineThreshold(fLine); //why don't I put these 2 together?
                if(ADCRead(Adc[0])>0.3f){ //approaching wall
                    SetMotor(Motors[0], 1.0f);
                    SetMotor(Motors[1], 1.0f);
                    while(ADCRead(Adc[0])<0.5f){} //not too close
                    float speed = 1.0f;
                    while(ADCRead(speed>0.0f){
                        SetMotor(Motors[0], SATURATE(0.0f,speed,1.0f));
                        SetMotor(Motors[1], SATURATE(0.0f,speed,1.0f));
                        speed = speed-0.2f;
                    } //pseudo deccelearte to score
                    statePos = 7; //RETURN
                    float time1 = GetTime();
                    SetMotor(Motors[0],-1.0f);
                    SetMotor(Motors[1],-1.0f);
                    while(GetTime()-time1<0.2f){}//back up
                    time1=GetTime();
                    SetMotor(Motors[0],-1.0f);
                    SetMotor(Motors[1],1.0f);
                    while(GetTime()-time1<0.3f){}//turn around 180
                    moveGate(); //open for marbles
                }
                else
                    LineFollow(fLine);
                break;
                
            case 7:
                LineSensorReadArray(fLine);
                LineThreshold(fLine); //I literally do this every time
                if(ADCRead(Adc[0])>0.3f){ //made it to wall
                    float speed = 1.0f;
                    while(ADCRead(speed>0.0f){
                        SetMotor(Motors[0], SATURATE(0.0f,speed,1.0f));
                        SetMotor(Motors[1], SATURATE(0.0f,speed,1.0f));
                        speed = speed-0.2f;
                    } //pseudo deccelearte to score    
                    float time1 = GetTime();
                    SetMotor(Motors[0],-1.0f);
                    SetMotor(Motors[1],-1.0f);
                    while(GetTime()-time1<0.2f){}//back up
                    time1=GetTime();
                    SetMotor(Motors[0],-1.0f);
                    SetMotor(Motors[1],1.0f);
                    while(GetTime()-time1<0.3f){}//turn around 180
                    statePos= 0 //reset to START;
                }
                else if(intersectCheck(fLine)){
                    float time1 = GetTime();
                    SetMotor(Motors[0], 1.0f);
                    SetMotor(Motors[1], 1.0f);
                    while(GetTime()-time1<0.2f){}
                }
                else
                    LineFollow();
                break;
                
            default:
                SetPin(PIN_F1,true);
                break;
        }
    }
}
