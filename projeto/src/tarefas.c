#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "system_tm4c1294.h" // CMSIS-Core
#include "cmsis_os2.h" // CMSIS-RTOS
#include "driverlib/uart.h"
#include "inc/hw_memmap.h"
#include "utils/uartstdio.h"
#include "inc/hw_ints.h"
#include "driverlib/interrupt.h"
#include "UARTInit.h"       

#define FLAG_MENSAGEM 0x0001
#define FLAG_ELEVADOR_1 0x0010
#define FLAG_ELEVADOR_2 0x0100
#define FLAG_ELEVADOR_3 0x1000


  
osMutexId_t uart_id;
osThreadId_t elevator_1_id, elevator_2_id, elevator_3_id, uart_id;

char commandE[6], commandC[6], commandD[6];

void UART0_Handler(void) {
  uint32_t uart_status = UARTIntStatus(UART0_BASE, true);
  UARTIntClear(UART0_BASE, uart_status);
  
  osThreadFlagsSet(uart_id, 0x0001);
}

void send(char* answer) {
  int i;
  
  for(i = 0; i < strlen(answer); i++) {
    UARTCharPut(UART0_BASE, answer[i]);
  }
}

int strMap(char c){
  switch(c){
    case 'a': return 0;
    case 'b': return 1;
    case 'c': return 2;
    case 'd': return 3;
    case 'e': return 4;
    case 'f': return 5;
    case 'g': return 6;
    case 'h': return 7;
    case 'i': return 8;
    case 'j': return 9;
    case 'k': return 10;
    case '1': return 11;
    case 'm': return 12;
    case 'n': return 13;
    case 'o': return 14;
    case 'p': return 15;
    default: return 0;
  }
}

char nbrMap(uint8_t n){
  switch(n){
    case 0: return 'a';
    case 1: return 'b';
    case 2: return 'c';
    case 3: return 'd';
    case 4: return 'e';
    case 5: return 'f';
    case 6: return 'g';
    case 7: return 'h';
    case 8: return 'i';
    case 9: return 'j';
    case 10: return 'k';
    case 11: return 'l';
    case 12: return 'm';
    case 13: return 'n';
    case 14: return 'o';
    case 15: return 'p';
    default: return 'a';
  }
}

//----------
// Elevator commands
void startElevator(char elevator) {
  char request[3] = {elevator, 'r', 0x0D};
  send(request);
}

void openElevator(char elevator) {
  char request[3] = {elevator, 'a', 0x0D};
  send(request);
}

void closeElevator(char elevator) {
  char request[3] = {elevator, 'f', 0x0D};
  send(request);
}

void goUp(char elevator) {
  char request[3] = {elevator, 's', 0x0D};
  send(request);
}

void goDown(char elevator) {
  char request[3] = {elevator, 'd', 0x0D};
  send(request);
}

void stopElevator(char elevator) {
  char request[3] = {elevator, 'p', 0x0D};
  send(request);
}

void turnLightOn(char elevator, char andar) {
  char request[4] = {elevator, 'L', andar, 0x0D};
  send(request);
}

void turnLightOff(char elevator, char andar) {
  char request[4] = {elevator, 'D', andar, 0x0D};
  send(request);
}

void thread_elevator_e(void *arg){
  char command[6];
  char state;
  bool targets[16];
  char targetDirection[16];
  bool lastCall, doorClosed;
  uint8_t actualFloor, targetFloor, first,second;
  int i;
  
  startElevator('e');
  closeElevator('e');
  
  state = 'p';
  doorClosed = false;
  actualFloor = 0;
  for(i = 0; i < 16 ; i++){
    targets[i] = false;
  }
  for(i = 0; i < 16 ; i++){
    targetDirection[i] = false;
  }
  
  osThreadFlagsClear(0x0001);
  osThreadFlagsSet(uart_id, FLAG_ELEVADOR_1);
  
  
  while(1){
    osThreadFlagsWait(0x0001, osFlagsWaitAny, osWaitForever);
    for(i = 0; i < 5 ; i++){
      command[i] = commandE[i];
      //printf("%c", command[i]);
    }
    //printf("\n");
    if((command[1]-'0') == 1) {
      actualFloor = ((command[0]-'0')* 10 + (command[1]-'0'));
    } else if ((command[0]-'0') <= 9 && (command[0]-'0') > 0 ){
      actualFloor = (command[0]-'0');
    }
    //printf("%d", actualFloor);
    //printf("%c", state);
    lastCall = true;
    if(actualFloor != 0 && actualFloor != 15) {
      for(i = actualFloor+1; i < 16; i++) {
        if(targets[i] == true) { 
          lastCall = false;
        }
      }
      for(i = 0; i < actualFloor; i++) {
        if((targets[i] == true)){
           lastCall = false;
        }
      }
    }
    if((targets[actualFloor] == true && targetDirection[actualFloor] == state) || lastCall == true ){
      stopElevator('e');
      state = 'p'; 
      targets[actualFloor] = false;
      targetDirection[actualFloor] = 'p';
      lastCall = false;
      if(doorClosed == true) {
        openElevator('e');
        osDelay(100000);
        closeElevator('e');
      }
      doorClosed = !doorClosed;
      //printf("%d",nbrMap(actualFloor));
      turnLightOff('e', nbrMap(actualFloor));
    }
    if(command[0] == 'A'){
      doorClosed = false;
    }
    if(command[0] == 'F'){
      doorClosed = true;
    }
    if(command[0] == 'E'){
      first = command[1] - '0';
      second = command[2] - '0';
      targetFloor = (first*10 + second);
      targets[targetFloor] = true;
      //printf("andar marcado: %d", targetFloor);
      if(command[3] == 's') {
        targetDirection[targetFloor] = 's';
      } else {
        targetDirection[targetFloor] = 'd';
      }
    }
    if(command[0] == 'I'){
      turnLightOn('e',command[1]);
      targets[strMap(command[1])] = true;
      if(actualFloor > strMap(command[1])) {
        targetDirection[targetFloor] = 'd';
      }
      else if(actualFloor < strMap(command[1])) {
        targetDirection[targetFloor] = 's';
      }
      else if(actualFloor == strMap(command[1])) {
        turnLightOff('e',command[1]);
        targets[strMap(command[1])] = false;
      }
    }
    for(i = actualFloor; i < 16; i++) {
      if(targets[i] == true && state != 'd') { 
        goUp('e');
        state = 's';
        break;
      }
    }
    for(i = 0; i < actualFloor; i++) {
      if((targets[i] == true) && state != 's'){
         goDown('e');
         state = 'd';
         break;
      }
    }
      
    for(i = 0; i < 6; i++) {
      command[i] = 0x0D;
    }
    osThreadFlagsClear(0x0001);
    osThreadFlagsSet(uart_id, FLAG_ELEVADOR_1);
  }
}

void thread_elevator_c(void *arg){
  char command[6];
  char state;
  bool targets[16];
  char targetDirection[16];
  bool lastCall, doorClosed;
  uint8_t actualFloor, targetFloor, first,second;
  int i;
  
  startElevator('c');
  closeElevator('c');
  
  state = 'p';
  doorClosed = false;
  actualFloor = 0;
  for(i = 0; i < 16 ; i++){
    targets[i] = false;
  }
  for(i = 0; i < 16 ; i++){
    targetDirection[i] = false;
  }
  osThreadFlagsClear(0x0001);
  osThreadFlagsSet(uart_id, FLAG_ELEVADOR_2);
  
  while(1){
    osThreadFlagsWait(0x0001, osFlagsWaitAny, osWaitForever);
        for(i = 0; i < 5 ; i++){
      command[i] = commandC[i];
      //printf("%c", command[i]);
    }
    //printf("\n");
    if((command[1]-'0') == 1) {
      actualFloor = ((command[0]-'0')* 10 + (command[1]-'0'));
    } else if ((command[0]-'0') <= 9 && (command[0]-'0') > 0 ){
      actualFloor = (command[0]-'0');
    }
    //printf("%d", actualFloor);
    //printf("%c", state);
    lastCall = true;
    if(actualFloor != 0 && actualFloor != 15) {
      for(i = actualFloor+1; i < 16; i++) {
        if(targets[i] == true) { 
          lastCall = false;
        }
      }
      for(i = 0; i < actualFloor; i++) {
        if((targets[i] == true)){
           lastCall = false;
        }
      }
    }
    if((targets[actualFloor] == true && targetDirection[actualFloor] == state) || lastCall == true ){
      stopElevator('c');
      state = 'p'; 
      targets[actualFloor] = false;
      targetDirection[actualFloor] = 'p';
      lastCall = false;
      if(doorClosed == true) {
        openElevator('c');
        osDelay(100000);
        closeElevator('c');
      }
      doorClosed = !doorClosed;
      //printf("%d",nbrMap(actualFloor));
      //turnLightOff('e', nbrMap(actualFloor));
    }
    if(command[0] == 'A'){
      doorClosed = false;
    }
    if(command[0] == 'F'){
      doorClosed = true;
    }
    if(command[0] == 'E'){
      first = command[1] - '0';
      second = command[2] - '0';
      targetFloor = (first*10 + second);
      targets[targetFloor] = true;
      //printf("andar marcado: %d", targetFloor);
      if(command[3] == 's') {
        targetDirection[targetFloor] = 's';
      } else {
        targetDirection[targetFloor] = 'd';
      }
    }
    if(command[0] == 'I'){
      //turnLightOn('e',command[1]);
      targets[strMap(command[1])] = true;
      if(actualFloor > strMap(command[1])) {
        targetDirection[targetFloor] = 'd';
      }
      else if(actualFloor < strMap(command[1])) {
        targetDirection[targetFloor] = 's';
      }
      else if(actualFloor == strMap(command[1])) {
        //turnLightOff('e',command[1]);
        targets[strMap(command[1])] = false;
      }
    }
    for(i = actualFloor; i < 16; i++) {
      if(targets[i] == true && state != 'd') { 
        goUp('c');
        state = 's';
        break;
      }
    }
    for(i = 0; i < actualFloor; i++) {
      if((targets[i] == true) && state != 's'){
         goDown('c');
         state = 'd';
         break;
      }
    }
      
    for(i = 0; i < 6; i++) {
      command[i] = 0x0D;
    }
    osThreadFlagsClear(0x0001);
    osThreadFlagsSet(uart_id, FLAG_ELEVADOR_2);
  }
}

void thread_elevator_d(void *arg){
  char command[6];
  char state;
  bool targets[16];
  char targetDirection[16];
  bool lastCall, doorClosed;
  uint8_t actualFloor, targetFloor, first,second;
  int i;
  
  startElevator('d');
  closeElevator('d');
  
  state = 'p';
  doorClosed = false;
  actualFloor = 0;
  for(i = 0; i < 16 ; i++){
    targets[i] = false;
  }
  for(i = 0; i < 16 ; i++){
    targetDirection[i] = false;
  }
  osThreadFlagsClear(0x0001);
  osThreadFlagsSet(uart_id, FLAG_ELEVADOR_3);
  while(1){
    osThreadFlagsWait(0x0001, osFlagsWaitAny, osWaitForever);
        for(i = 0; i < 5 ; i++){
      command[i] = commandD[i];
      //printf("%c", command[i]);
    }
    //printf("\n");
    if((command[1]-'0') == 1) {
      actualFloor = ((command[0]-'0')* 10 + (command[1]-'0'));
    } else if ((command[0]-'0') <= 9 && (command[0]-'0') > 0 ){
      actualFloor = (command[0]-'0');
    }
    //printf("%d", actualFloor);
    //printf("%c", state);
    lastCall = true;
    if(actualFloor != 0 && actualFloor != 15) {
      for(i = actualFloor+1; i < 16; i++) {
        if(targets[i] == true) { 
          lastCall = false;
        }
      }
      for(i = 0; i < actualFloor; i++) {
        if((targets[i] == true)){
           lastCall = false;
        }
      }
    }
    if((targets[actualFloor] == true && targetDirection[actualFloor] == state) || lastCall == true ){
      stopElevator('d');
      state = 'p'; 
      targets[actualFloor] = false;
      targetDirection[actualFloor] = 'p';
      lastCall = false;
      if(doorClosed == true) {
        openElevator('d');
        osDelay(100000);
        closeElevator('d');
      }
      doorClosed = !doorClosed;
      //printf("%d",nbrMap(actualFloor));
      //turnLightOff('e', nbrMap(actualFloor));
    }
    if(command[0] == 'A'){
      doorClosed = false;
    }
    if(command[0] == 'F'){
      doorClosed = true;
    }
    if(command[0] == 'E'){
      first = command[1] - '0';
      second = command[2] - '0';
      targetFloor = (first*10 + second);
      targets[targetFloor] = true;
      //printf("andar marcado: %d", targetFloor);
      if(command[3] == 's') {
        targetDirection[targetFloor] = 's';
      } else {
        targetDirection[targetFloor] = 'd';
      }
    }
    if(command[0] == 'I'){
      //turnLightOn('e',command[1]);
      targets[strMap(command[1])] = true;
      if(actualFloor > strMap(command[1])) {
        targetDirection[targetFloor] = 'd';
      }
      else if(actualFloor < strMap(command[1])) {
        targetDirection[targetFloor] = 's';
      }
      else if(actualFloor == strMap(command[1])) {
        //turnLightOff('e',command[1]);
        targets[strMap(command[1])] = false;
      }
    }
    for(i = actualFloor; i < 16; i++) {
      if(targets[i] == true && state != 'd') { 
        goUp('d');
        state = 's';
        break;
      }
    }
    for(i = 0; i < actualFloor; i++) {
      if((targets[i] == true) && state != 's'){
         goDown('d');
         state = 'd';
         break;
      }
    }
      
    for(i = 0; i < 6; i++) {
      command[i] = 0x0D;
    }
    osThreadFlagsClear(0x0001);
    osThreadFlagsSet(uart_id, FLAG_ELEVADOR_3);
  }
}

void thread_uart(void *arg){
  char character;
  int i = 0;
  while(1) {
    osThreadFlagsWait(0x0001, osFlagsWaitAny, osWaitForever);
    while(UARTCharsAvail(UART0_BASE)) {
      character = UARTCharGet(UART0_BASE);
     
      for(i = 0; i < 6; i++) {
        commandE[i] = 0x0D;
        commandC[i] = 0x0D;
        commandD[i] = 0x0D;
      }
      i = 0;
      if(character == 'e') {
        while(character != 0x0D && character != 0x0A) {
          character = UARTCharGet(UART0_BASE);
          commandE[i] = character;
          //printf("%c", character);
          i++;
        }
        //printf("\n");
        osThreadFlagsSet(elevator_1_id, 0x0001);
        osThreadFlagsWait(FLAG_ELEVADOR_1, osFlagsWaitAny, osWaitForever);
        osThreadFlagsClear(FLAG_ELEVADOR_1);
      }
      
      else if(character == 'c') {
        while(character != 0x0D && character != 0x0A) {
          character = UARTCharGet(UART0_BASE);
          commandC[i] = character;
          i++;
        }
        osThreadFlagsSet(elevator_2_id, 0x0001);
        osThreadFlagsWait(FLAG_ELEVADOR_2, osFlagsWaitAny, osWaitForever);
        osThreadFlagsClear(FLAG_ELEVADOR_2);
      }
      
      else if(character == 'd') {
        while(character != 0x0D && character != 0x0A) {
          character = UARTCharGet(UART0_BASE);
          commandD[i] = character;
          i++;
        }
        osThreadFlagsSet(elevator_3_id, 0x0001);
        osThreadFlagsWait(FLAG_ELEVADOR_3, osFlagsWaitAny, osWaitForever);
        osThreadFlagsClear(FLAG_ELEVADOR_3);
      }
    }
    osThreadFlagsClear(0x0001);
  }
}

const osThreadAttr_t thread1_attr = {
  .stack_size = 1024                            // Create the thread stack with a size of 1024 bytes
};

void main(void){
  UARTInit();
  
  UARTConfigSetExpClk(UART0_BASE, SystemCoreClock, 115200, (UART_CONFIG_PAR_NONE | UART_CONFIG_STOP_ONE | UART_CONFIG_WLEN_8));
  UARTFIFOLevelSet(UART0_BASE, UART_FIFO_TX1_8, UART_FIFO_RX1_8);
  UARTIntDisable(UART0_BASE, 0xFFFFFFFF);
  UARTIntEnable(UART0_BASE, UART_INT_RX);
  IntEnable(INT_UART0);
  UARTEnable(UART0_BASE);
  
  if(osKernelGetState() == osKernelInactive)
     osKernelInitialize();

  elevator_1_id = osThreadNew(thread_elevator_e, NULL, &thread1_attr);
  while(elevator_1_id == NULL) {
    elevator_1_id = osThreadNew(thread_elevator_e, NULL, &thread1_attr);
  }
  
  elevator_2_id = osThreadNew(thread_elevator_c, NULL, &thread1_attr);
  while(elevator_2_id == NULL) {
    elevator_2_id = osThreadNew(thread_elevator_c, NULL, &thread1_attr);
  }
  
  elevator_3_id = osThreadNew(thread_elevator_d, NULL, &thread1_attr);
  while(elevator_3_id == NULL) {
    elevator_3_id = osThreadNew(thread_elevator_d, NULL, &thread1_attr);
  }
  
  uart_id = osThreadNew(thread_uart, NULL, NULL);
  while(uart_id == NULL) {
    uart_id = osThreadNew(thread_uart, NULL, NULL);
  }
  if(osKernelGetState() == osKernelReady)
    osKernelStart();

  while(1);
} // main
