#include <stdbool.h>
#include "system_tm4c1294.h" // CMSIS-Core
#include "driverleds.h" // device drivers
#include "driverbuttons.h"
#include "cmsis_os2.h" // CMSIS-RTOS
#include "driverlib/interrupt.h"
#include "inc/hw_ints.h"

#define CHECK_BIT(var,pos) !!((var) & (1<<(pos)))

void setLeds(int valor){
  if(CHECK_BIT(valor,0)) LEDOn(LED4); else LEDOff(LED4);
  if(CHECK_BIT(valor,1)) LEDOn(LED3); else LEDOff(LED3);
  if(CHECK_BIT(valor,2)) LEDOn(LED2); else LEDOff(LED2);
  if(CHECK_BIT(valor,3)) LEDOn(LED1); else LEDOff(LED1);
}

osThreadId_t thread1_id;
int count = 0;

void thread1(void *arg){
 
  while(1){
    setLeds(count);
    osDelay(100);
  } 
}

void GPIOJ_Handler() {

    count++;
    ButtonIntClear(USW1);
}

void main(void){
  LEDInit(LED2 | LED1 | LED3 | LED4);
  ButtonInit(USW1);
  ButtonIntEnable(USW1);
  ButtonIntClear(USW1);
  
  osKernelInitialize();

  thread1_id = osThreadNew(thread1, NULL, NULL);

  if(osKernelGetState() == osKernelReady)
    osKernelStart();

  while(1);
} // main

