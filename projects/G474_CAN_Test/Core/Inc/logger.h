#ifndef LOGGER
#define LOGGER

#define MessageQSize 16
#define MAX_MESSAGE_SIZE 256
#define LOG_DEBUG 0
#define LOG_INFO 1
#define LOG_ERROR 2
#define LOG_CRITICAL 3 // Blocking, will log instantly

#define LOG_FROM_ISR 16 // Or it with the proprer log type
#define LOG_CM7      32
#define LOG_PLOT     64 // only timestamp added  

#define LOG_ARR_HELPER_12(a) a[0], a[1], a[2], a[3], a[4], a[5], a[6], a[7], a[8], a[9], a[10], a[11]
#define LOG_ARR_HELPER_5(a)  a[0], a[1], a[2], a[3], a[4]


#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "usart.h"
#include "stdbool.h"

typedef struct {
  uint8_t len;                   // actual data length
  char data[MAX_MESSAGE_SIZE];   // command payload
} Message_t;

extern volatile bool debug_enabled; // used to enable or disable debug printing



void logger(int importance,char* message,...);
void loggerRAW(int importance, char* message,size_t len); //Sends the message raw without any additional stuff.
void loggerInit(bool debug_en,UART_HandleTypeDef* huart);

#endif