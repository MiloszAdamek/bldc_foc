#ifndef UART_DMA
#define UART_DMA

#include "stm32g4xx_hal.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"
#include <string.h>
#include <stdint.h>

#define MAX_CMD_LEN   64    // maximum bytes in one command
#define CMD_QUEUE_LEN 10    // max number of pending commands in queue

#define COMMAND_PARAMS 5 		// amount of supported params (single char)
#define COMMAND_PARAM_LEN 16	// How much space is needed for the param value
#define COMMAND_NAME_LEN  16	// How much space is needed for the command type

// The command should be written as follows: 
// <name> =<param name> <value> =<next param> <next value>
// For example
// set =P 10.2 =I 0.2 
// Could be used to set P and I params of a PI controller 

typedef struct {
    uint8_t len;                 // actual data length
    char data[MAX_CMD_LEN];   // command payload
} Command_t;

// Everything will be \0 terminated
typedef struct {
	char name[COMMAND_NAME_LEN];
	char params[COMMAND_PARAMS];
	char values[COMMAND_PARAMS][COMMAND_PARAM_LEN];
} Processed_Command_t;



typedef struct
{
	UART_HandleTypeDef* huart;			// UART handler

	uint8_t DMA_RX_Buffer[MAX_CMD_LEN];	// DMA direct buffer

	QueueHandle_t queue_commands;
	Command_t curr_command;
}UARTDMA_HandleTypeDef;

// To use this command simply Init it in main and implement this callback
/*
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size){
	if(huart->Instance ==USART1){
		UARTDMA_IrqHandler(&myUARTDMA);
	}

*/

void UARTDMA_IrqHandler(UARTDMA_HandleTypeDef *huartdma);

void UARTDMA_Init(UARTDMA_HandleTypeDef *huartdma, UART_HandleTypeDef *huart);

void Process_Command(Command_t* command, Processed_Command_t* processed_command);
#endif