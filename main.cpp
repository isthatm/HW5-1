#include "mbed.h"
#include "bbcar.h"
#include <cmath>
#include <cstdio>

DigitalOut led1(LED1);
Ticker servo_ticker;
Ticker servo_feedback_ticker;
Thread carThread;
Thread measureThread;
EventQueue car_queue;
EventQueue measure_queue;

//Car
PwmIn servo0_f(D12), servo1_f(D11); // servo0 - left ; servo1 - right
PwmOut servo0_c(D9), servo1_c(D10); 
BBCar car(servo0_c, servo0_f, servo1_c, servo1_f, servo_ticker, servo_feedback_ticker);

//Laser ping
DigitalInOut pin8(D8);
parallax_ping  ping1(pin8);

// QTI sensors
BusInOut qti_pin(D4,D5,D6,D7);
parallax_qti qti1(qti_pin);
int pattern;
int currentAngle;

const float PI = 3.13159;
const float R = 5.1; //distance between wheels -> radius of rotaion
const float rotP = 32.04424507;
float obj[2], w[2];
int i = 0;
int rotatingSpeed = 45;

int obsCaution = 1;


void widthCalculation (double d, double currentWheelAngle, double initWheelAngle ){
    float wheelAngle, rotAngle, p, theta;
    wheelAngle =  abs(currentWheelAngle) - abs(initWheelAngle);
    p = 6.5*PI*( abs(wheelAngle) / 360 );
    theta = (p / rotP)*360; //
    //alpha1 = 90 - theta1;
    rotAngle = (theta*PI) / 180; // deg to rad
    w[i] = sin(rotAngle)*d;
    i++;
}

void obsMeasure1(){
    double d1, d2, obsMeasureAngle0;
    obsMeasureAngle0 = car.servo1.angle;
    while(1) {
        d1 = (float)ping1;
        if (d1 > 20){
            printf("d1: %f\n", d1);
            car.rotate(rotatingSpeed);
        }
        else{
            car.stop();
            break;
        }
        ThisThread::sleep_for(10ms);
    }
    currentAngle = car.servo1.angle;
    widthCalculation(d1, currentAngle, obsMeasureAngle0);
    printf("Distance to object 1: %f , w1: %f\n", d1, w[0]);
    
    // 2nd object
    while(1) {
        pattern = (int)qti1;
        if (pattern == 0b0110){
            car.stop();
            ThisThread::sleep_for(1s);
            obsMeasureAngle0 = car.servo1.angle; 
            break;
        }
        else {
            car.rotate(-rotatingSpeed);
        }
    }
    while(1){
        d2 = (float)ping1;
        if (d2 > 20){
            printf("d2: %f\n", d2);
            car.rotate(-rotatingSpeed);
        }
        else{
            car.stop();
            break;
        }
        ThisThread::sleep_for(10ms);
    }
    obsCaution = 1;
    
    widthCalculation(d2 , car.servo1.angle, obsMeasureAngle0);
    printf("Distance to object 2: %f , w2: %f\n", d2, w[1]);
    printf("Distance between 2 objects: %f\n", w[0] + w[1]);
    
    while(1){
       pattern = (int)qti1;
       if (pattern == 0b0110){
           car.stop();
           break;
       }
       else{
           car.rotate(rotatingSpeed);
       }
   }
}

int main() {
    carThread.start(callback(&car_queue, &EventQueue::dispatch_forever));
    car_queue.call(&obsMeasure1); 
}

