//main.c
//authored by Jared Hull
//
//tasks 1 and 2 blink the ACT LED
//main initialises the devices and IP tasks

#include <FreeRTOS.h>
#include <task.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <queue.h>
#include <math.h>
#include "interrupts.h"
#include "gpio.h"
#include "video.h"
#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"

#define WHITE 0xFFFFFFFF
#define RED 0Xff0000
#define GREEN 0X00ff12
#define LIGHTBLUE 0X00baff
#define YELLOW 0Xffe400
#define GREY 0Xd4d4d4
#define DARKGREY 0X363333
#define BLUE 0X001eff
#define BLACK 0X000000

xQueueHandle waterQ;
xQueueHandle graphWatQ;
xQueueHandle graphVarQ;

void task1() {
	while(1) {
		SetGpio(47, 1);
		vTaskDelay(200);
                SetGpio(47,0);
                vTaskDelay(100);
	}
}

void waterGenerator(){
#define OVERPRESSURE 200    
    int r;
    int value;
    char str[15];
    println("Water generator started!",LIGHTBLUE);   
    while(1){
        r=rand();
        value = OVERPRESSURE+1000 + (r % 200) + 1 - 100; //Between -100 and 100

        if(waterQ != 0){
            if( xQueueSendToBack( waterQ,( void * ) &value, 0 ) != pdPASS ){
                println("Error could not send to Water Queue!", RED);
            }
            else{
                //println("Data sent to queue", GREEN);
            }
        }

        vTaskDelay(150);
    }
}

void waterController(){
    /* constants for PID */
    float Kp = 0.9;
    float Ki = 0.05;
    float Kd = 0.05;
    float SetPoint = 1000.0;
    float d_Temp = 0.0;
    float i_Temp = 0.0;
    float error = 0.0;
    float iMax = 200.0;
    float iMin = -200.0;
    float P_Term = 0.0;
    float I_Term = 0.0;
    float D_Term = 0.0;  
    float out = 0.0;
    int waterpressure = 0;
    int buf = 0;    
    char debug[15];   
    println("Water controller started!",LIGHTBLUE);
    
    while(1){      
        if(waterQ != 0){
            if(xQueueReceive(waterQ,&buf, 0) == pdPASS){
#if 0
                itoa(buf, debug, 10);
                println(debug,GREEN);
#endif
        
                error = SetPoint - (float)buf;
                P_Term = Kp * error; //Set P
                i_Temp += error;
                if (i_Temp > iMax){
                    i_Temp = iMax;
                }       
                else if (i_Temp < iMin){
                    i_Temp = iMin;
                    
                }
      
                I_Term = Ki * i_Temp; //Set I
                D_Term = Kd * (d_Temp - error); //Set D
                d_Temp = error;
                out = P_Term+I_Term+D_Term;

                waterpressure = buf + (int)out;


                if( xQueueSendToBack( graphWatQ,( void * ) &waterpressure, 0 ) != pdPASS ){
                    println("Error could not send to Graph Water Queue!", RED);
                }
                else{
                
                }

                if( xQueueSendToBack( graphVarQ,( void * ) &out, 0 ) != pdPASS ){
                    println("Error could not send to Graph Var Queue!", RED);
                }
                else{
                
                }

            }
            else{
                //Noting in water queue
            }
        }

        vTaskDelay(120);
    }
}

void graphWater(){
    #define XOFFSETWAT 1000
    #define YOFFSETWAT 50
    #define POINTS 100
    char debug[15]; 
    int buf = 0;
    int xpoint = 0;
    int xpoints[POINTS];
    int i;
    for(i=0;i<POINTS;i++)
        xpoints[i]=YOFFSETWAT;

    while(1){
        if(graphWatQ != 0){
            if(xQueueReceive(graphWatQ,&buf, 10) == pdPASS){
                buf *= 0.5; //Scale down by 50%
                if(xpoint >= POINTS){
                    xpoint = POINTS-1;
                    for(i=0;i<POINTS-1;i++)
                        xpoints[i]=xpoints[i+1];
                    xpoints[POINTS-1] = buf;
                }
                else{
                    xpoints[xpoint] = buf;
                }

                if(xpoint >= 1){
                    for(i=0;i<xpoint;i++){
                        drawLine(2*i+XOFFSETWAT,xpoints[i]+YOFFSETWAT,2*i+2+XOFFSETWAT,xpoints[i+1]+YOFFSETWAT,RED);
                    }
                float theta = ((float)buf*2/1400.0)*360.0; //the angle in degrees                
                float thetarad = theta * 3.1415 / 180.0;
                float x = 1300.0+(45.0*sinf(thetarad))*-1;
                float y = 550.0+(45.0*cosf(thetarad));
                drawLine(1300,550,(int)x,(int)y,WHITE); 

                }
                
                vTaskDelay(100);
                
                if(xpoint >= 1){
                    for(i=0;i<xpoint;i++){
                        drawLine(2*i+XOFFSETWAT,xpoints[i]+YOFFSETWAT,2*i+2+XOFFSETWAT,xpoints[i+1]+YOFFSETWAT,DARKGREY);
                    }
                }
                
                float theta = ((float)buf*2/1400.0)*360.0; //the angle in degrees                
                float thetarad = theta * 3.1415 / 180.0;
                float x = 1300.0+(45.0*sinf(thetarad))*-1;
                float y = 550.0+(45.0*cosf(thetarad));
                drawLine(1300,550,(int)x,(int)y,BLACK);     

                xpoint++;
            }
        }            
    }
}

void graphVariable(){
    #define XOFFSETVAR 1000
    #define YOFFSETVAR 300
    char debug[15]; 
    float buf = 0;
    int xpoint = 0;
    int xpoints[POINTS];
    int i;
    int change = 0;
    for(i=0;i<POINTS;i++)
        xpoints[i]=YOFFSETVAR;
    
    while(1){
        if(graphVarQ != 0){
            if(xQueueReceive(graphVarQ,&buf, 10) == pdPASS){
                buf *= 0.2;
                if(xpoint >= POINTS){
                    xpoint = POINTS-1;
                    for(i=0;i<POINTS-1;i++)
                        xpoints[i]=xpoints[i+1];
                    xpoints[POINTS-1] = buf;
                }
                else{
                    xpoints[xpoint] = buf;
                }

                if(xpoint >= 1){
                    for(i=0;i<xpoint;i++){
                        drawLine(2*i+XOFFSETVAR,xpoints[i]+YOFFSETVAR,2*i+2+XOFFSETVAR,xpoints[i+1]+YOFFSETVAR,GREEN);
                    }
#if 0
                itoa(buf, debug, 10);
                println(debug,GREEN);
#endif
                    
                }
                vTaskDelay(100);
                if(xpoint >= 1){
                    for(i=0;i<xpoint;i++){
                        drawLine(2*i+XOFFSETVAR,xpoints[i]+YOFFSETVAR,2*i+2+XOFFSETVAR,xpoints[i+1]+YOFFSETVAR,DARKGREY);
                    }
                }
                xpoint++;
            }
        }            
    }
}


void sporadicTask(){
    long i,j;
    vTaskDelay(5000);
    println("Sporadic task arrived!", YELLOW);
    while(1){
        for(i=0;i<100000000;i++)
            vTaskDelay(1);
            for(j=0;j<10000000000;j++)
                ;
    }
}

int main(void) {
	SetGpioFunction(47, 1);			// RDY led
        char debug[15];
	initFB();
	SetGpio(47, 1);
	//videotest();
        println("FreeRTOS started!", GREEN);
	DisableInterrupts();
	InitInterruptController();
        //EnableInterrupts();       

        drawString("Control variable",1000,210,LIGHTBLUE);
        drawString("-150",1000-25,220,LIGHTBLUE);
        drawString("0",1000-10,295,LIGHTBLUE);
        drawString("150",1000-20,370,LIGHTBLUE);       
        drawRect(1000,220,1200,380,DARKGREY);
        
        drawString("Water pressure",1000,490,LIGHTBLUE);
        drawString("800",1000-20,500,LIGHTBLUE);
        drawString("1000",1000-30,545,LIGHTBLUE);
        drawString("1200",1000-30,590,LIGHTBLUE);      
        drawRect(1000,500,1200,600,DARKGREY);        
        drawCircle(1300,550,50,BLUE);
        drawCircle(1300,550,49,BLUE);
        drawCircle(1300,550,48,BLUE);
        drawString("1400",1300+25,550+45,RED);
        drawString("1200",1300+55,550-5,WHITE);
        drawString("950",1300+45,550-40,WHITE);
        drawString("700",1300-5,550-60,GREEN);
        drawString("525",1300-55,550-40,GREEN);
        drawString("350",1300-70,550-5,GREEN);
        drawString("0",1300-45,550+45,GREEN);
        
        
        //Y värdet har cos
        //X värdet har sin

        waterQ = xQueueCreate( 10, sizeof( unsigned int ) );
        graphVarQ = xQueueCreate( 10, sizeof( float ) );
        graphWatQ = xQueueCreate( 10, sizeof( unsigned int ) );
        
        //The higher number the higher priority
	xTaskCreate(task1, "LED_0", 128, NULL, 1, NULL);
        xTaskCreate(waterController, "WaterController", 512, NULL, 1, NULL);
        xTaskCreate(waterGenerator, "WaterGenerator", 256, NULL, 3, NULL);
        xTaskCreate(graphWater, "GraphWater", 512, NULL, 2, NULL);
        xTaskCreate(graphVariable, "graphVariable", 512, NULL, 2, NULL);
        xTaskCreate(sporadicTask, "sporadicTask", 128, NULL, 4, NULL); 
        
        
        

	//set to 0 for no debug, 1 for debug, or 2 for GCC instrumentation (if enabled in config)
	loaded = 1;

	vTaskStartScheduler();

	/*
	 *	We should never get here, but just in case something goes wrong,
	 *	we'll place the CPU into a safe loop.
	 */
	while(1) {
		;
	}
}
