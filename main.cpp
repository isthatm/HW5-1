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
PwmIn servo0_f(D12), servo1_f(D11); // servo0 - left ; servo1 - right
PwmOut servo0_c(D9), servo1_c(D10); 

DigitalInOut pin8(D8);

BBCar car(servo0_c, servo0_f, servo1_c, servo1_f, servo_ticker, servo_feedback_ticker);

float width;
float nonObjectTable [2];
float objectTable[2];
int i1 = 0; int i2 = 0;

//distance to objects 20cm
// distance between two objects

void obsMeasure(){
    float distance;
    parallax_ping  ping1(pin8);
    while(1) {
        
        distance = (float)ping1;
        printf("distance = %f\n", distance);
        if(distance<25){
            objectTable[i1] = distance;
            if (i1 == 0){
                i2 = 1;
            }
            else if(i1 == 1){
                i1 = 2; // edege of the object is found
            }
        } 
        else {
            nonObjectTable[i2] = distance;
            if (i2 == 1){ 
                // impying that the last number in objectTable is the inner edge
                // increase i to store the distance of the 2nd objec innter edge
                i1=1;
            }
        }
        if (i1 == 2){
            break;
        }
        ThisThread::sleep_for(1s); 
   }
   
   width = sqrt((objectTable[0]*objectTable[0] + objectTable[1]*objectTable[1] )/2 - 19*19);
   printf("a = %f, b = %f, w = %f \n", objectTable[0], objectTable[1], width);                                
   
   
}

void rotate2Measure (){
    float dist = 7;
    float err = 0.7;
    car.rotateCertainDistance(-dist);
    while (car.checkRotateDistance(err)){
        ThisThread::sleep_for(100ms);
    }
    car.stop();
    measure_queue.call(&obsMeasure);
    ThisThread::sleep_for(2s);

    car.rotateCertainDistance(1.7*dist);
    while (car.checkRotateDistance(1.7*err)){
        ThisThread::sleep_for(100ms);
    }
    car.stop();
    ThisThread::sleep_for(2s);

    car.rotateCertainDistance(-dist);
    while (car.checkRotateDistance(err)){
        ThisThread::sleep_for(100ms);
    }
    car.stop();
    ThisThread::sleep_for(2s);
    
    
}

int main() {
    carThread.start(callback(&car_queue, &EventQueue::dispatch_forever));
    measureThread.start(callback(&measure_queue, &EventQueue::dispatch_forever));
    car_queue.call(&rotate2Measure);
}
