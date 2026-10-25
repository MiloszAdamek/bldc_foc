#include "UART_DMA.h"
#include "string.h"
#include <stdbool.h>
#include "logger.h"

void UARTDMA_IrqHandler(UARTDMA_HandleTypeDef *huartdma)
{
    uint8_t* DmaBufferPointer;
    uint32_t Length;
    uint16_t i;


	// Get number of received bytes by subtracting DMA counter from buffer size
	Length = MAX_CMD_LEN - __HAL_DMA_GET_COUNTER(huartdma->huart->hdmarx);
	DmaBufferPointer = huartdma->DMA_RX_Buffer;

	for(i = 0; i < Length; i++)
	{
		huartdma->curr_command.data[huartdma->curr_command.len] = DmaBufferPointer[i];

		if(DmaBufferPointer[i] == '\n')
		{
			Command_t new_command = { .len = 0 };
			BaseType_t xHigherPriorityTaskWoken = pdFALSE;
			xQueueSendFromISR(huartdma->queue_commands, &(huartdma->curr_command), &xHigherPriorityTaskWoken);
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

			huartdma->curr_command = new_command;
		}
		else
		{
			huartdma->curr_command.len++;
		}
	}

	HAL_UARTEx_ReceiveToIdle_DMA(huartdma->huart, huartdma->DMA_RX_Buffer, MAX_CMD_LEN);
	__HAL_DMA_DISABLE_IT(huartdma->huart->hdmarx, DMA_IT_HT);  // Disable Half Transfer interrupt
}

void UARTDMA_Init(UARTDMA_HandleTypeDef *huartdma, UART_HandleTypeDef *huart)
{
    huartdma->huart = huart;

    QueueHandle_t queue = xQueueCreate(CMD_QUEUE_LEN, sizeof(Command_t));
    configASSERT(queue);
    huartdma->queue_commands = queue;

    huartdma->curr_command.len = 0;
	
	HAL_UARTEx_ReceiveToIdle_DMA(huartdma->huart, huartdma->DMA_RX_Buffer, MAX_CMD_LEN);
    __HAL_DMA_DISABLE_IT(huartdma->huart->hdmarx, DMA_IT_HT);  // Disable Half Transfer interrupt
}

void Process_Command(Command_t* command, Processed_Command_t* processed_command){
	bool finding_params = false;
	size_t params_found = 0;
	size_t param_idx = 0;

	for (size_t i = 0; i < COMMAND_PARAMS;i++){
		processed_command->params[i] = '\0';
	}
	
	for(size_t i =0; i < command->len; i++){
		if(!finding_params){
			if(i >= COMMAND_NAME_LEN){
				logger(LOG_ERROR,"Error while processing command: the name is too long");
				break;
			}
			if(command->data[i] == ' '){
				finding_params = true;
				processed_command->name[i] = '\0';
				continue;
			}
			processed_command->name[i] = command->data[i];
		}else{
			if(command->data[i] == ' '){
				processed_command->values[params_found][param_idx] = '\0';
				params_found ++;
				param_idx = 0;
			}
			else if (command->data[i] == '='){
				if (command->len - i <= 2){
					logger(LOG_ERROR, "The command seems to be formatted incorrectly");
					break;
				}
				processed_command->params[params_found] = command->data[i+1];
				i+=2; // skip the param and the space
			}else{
				if (param_idx >= COMMAND_PARAM_LEN - 1){
					logger(LOG_ERROR, "Not enough space for parameter value");
					break;
				}
				processed_command->values[params_found][param_idx] = command->data[i];
				param_idx++;
			}
		}
	}
	if (!finding_params) processed_command->name[command->len] = '\0';
	processed_command->values[params_found][param_idx] = '\0';


}
