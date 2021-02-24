#include <stdint.h>
#include <stdbool.h>
// includes da biblioteca driverlib
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"

uint8_t LED_D4 = 0;

void main(void){

  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF); // Habilita GPIO F (LED D4 = PF0)
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF)); // Aguarda final da habilita��o
    
  GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_4); // LED D4 como sa�da
  GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, 0); // LED D4 apagado
  GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_STRENGTH_12MA, GPIO_PIN_TYPE_STD);

  while(1){
    for(int i = 0 ; i < 2400000 ; i++) {}
      LED_D4 ^= GPIO_PIN_0; // Troca estado do LED D1
      GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, LED_D4); // Acende LED D4
  } // while
} // main

/* 
  Basicamente ao trocar a otimiza��o do compilador C os ciclos ficam mais r�pidos. 
  Na configura��o Medium � poss�vel ver que o LED pisca mais r�pido e na op��o
  High o LED nem parece piscar, ele fica aceso por�m com uma luz mais apagada
  que o normal.

  Mudando para 120MHz, naturalmente os ciclos ficam mais r�pidos e h� um claro
  aumento na velocidade do LED piscando j� na configura��o Low.

*/