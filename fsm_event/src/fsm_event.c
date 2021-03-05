#include <stdint.h>
#include <stdbool.h>
// includes da biblioteca driverlib
#include "driverlib/systick.h"
#include "driverleds.h" // Projects/drivers

// MEF com apenas 2 estados e 1 evento temporal que alterna entre eles
// Seleção por evento


volatile uint8_t Evento = 0;

void SysTick_Handler(void){
  Evento = 1;
} // SysTick_Handler

void main(void){
  static int Estado = 000; // estado inicial da MEF
  
  LEDInit(LED1);
  LEDInit(LED2);
  LEDInit(LED3);
  SysTickPeriodSet(12000000); // f = 1Hz para clock = 24MHz
  SysTickIntEnable();
  SysTickEnable();

  while(1){
    if(Evento){
      Evento = 0;
      switch(Estado){
        case 000:
          LEDOff(LED1);
          LEDOff(LED2);
          LEDOff(LED3);
          Estado = 001;
          break;
        case 001:
          LEDOn(LED1);
          LEDOff(LED2);
          LEDOff(LED3);
          Estado = 011;
          break;
        case 011:
          LEDOn(LED1);
          LEDOn(LED2);
          LEDOff(LED3);
          Estado = 010;
          break;
        case 010:
          LEDOff(LED1);
          LEDOn(LED2);
          LEDOff(LED3);
          Estado = 110;
          break;
        case 110:
          LEDOff(LED1);
          LEDOn(LED2);
          LEDOn(LED3);
          Estado = 111;
          break;
        case 111:
          LEDOn(LED1);
          LEDOn(LED2);
          LEDOn(LED3);
          Estado = 101;
          break;
        case 101:
          LEDOn(LED1);
          LEDOff(LED2);
          LEDOn(LED3);
          Estado = 100;
          break;
        case 100:
          LEDOff(LED1);
          LEDOff(LED2);
          LEDOn(LED3);
          Estado = 000;
          break;
      } // switch
    } // if
  } // while
} // main
