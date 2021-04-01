#include <stdbool.h>
#include "system_tm4c1294.h" // CMSIS-Core
#include "driverleds.h" // device drivers
#include "driverbuttons.h"
#include "cmsis_os2.h" // CMSIS-RTOS
#include "driverlib/interrupt.h"
#include "inc/hw_ints.h"

#define CHECK_BIT(var,pos) !!((var) & (1<<(pos)))
#define BUFFER_SIZE 8

void setLeds(int valor){
  if(CHECK_BIT(valor,0)) LEDOn(LED4); else LEDOff(LED4);
  if(CHECK_BIT(valor,1)) LEDOn(LED3); else LEDOff(LED3);
  if(CHECK_BIT(valor,2)) LEDOn(LED2); else LEDOff(LED2);
  if(CHECK_BIT(valor,3)) LEDOn(LED1); else LEDOff(LED1);
}

osThreadId_t thread1_id, thread2_id;
osSemaphoreId_t semaphore1_id, semaphore2_id;
uint8_t buffer[BUFFER_SIZE];
uint32_t tick = 0;
int count = 0;
bool debounce = true;

void thread1(void *arg){
 
  while(1){
    osSemaphoreAcquire(semaphore1_id, osWaitForever);
    setLeds(count);
  } 
}

void thread2(void *arg){
 
  while(1){
    osSemaphoreAcquire(semaphore2_id, osWaitForever);
    tick = osKernelGetTickCount();
    osDelayUntil(tick + 500);
    debounce = true;
    osSemaphoreRelease(semaphore1_id);
  } 
}

void GPIOJ_Handler() {
    ButtonIntClear(USW1);
    if( debounce) {
      count++;
      debounce = false;
    }
    osSemaphoreRelease(semaphore2_id);
}

void main(void){
  LEDInit(LED2 | LED1 | LED3 | LED4);
  ButtonInit(USW1);
  ButtonIntEnable(USW1);
  ButtonIntClear(USW1);
  
  osKernelInitialize();

  thread1_id = osThreadNew(thread1, NULL, NULL);
  thread2_id = osThreadNew(thread2, NULL, NULL);
  semaphore1_id = osSemaphoreNew(BUFFER_SIZE, BUFFER_SIZE, NULL);

  if(osKernelGetState() == osKernelReady)
    osKernelStart();

  while(1);
} // main

