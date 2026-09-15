#include "logger.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "main.h"
#include "FreeRTOS.h"

Message_t curr_message; // Used by the logger function to temporary place data here before it is copied to the q.
Message_t sending_message_error;
Message_t sending_message_info;
Message_t sending_message_debug;

SemaphoreHandle_t xMutexUART1;

TaskHandle_t task_send_error;
TaskHandle_t task_send_info;
TaskHandle_t task_send_debug;

//Queues to send data via UART in a separate task without blocking
QueueHandle_t queue_debug;
QueueHandle_t queue_info;
QueueHandle_t queue_error;

UART_HandleTypeDef* huart;

volatile TaskHandle_t task_DMA_UART_Notify = NULL;

volatile bool debug_enabled;

static void send_debug(void* pvParameters);
static void send_info(void* pvParameters);
static void send_error(void* pvParameters);

void logger(int importance,char* message,...) {
    bool fromISR = importance&LOG_FROM_ISR;
    bool CM7 = importance&LOG_CM7;
    bool plot = importance&LOG_PLOT;
    importance = importance&~LOG_FROM_ISR;
    importance = importance&~LOG_CM7;
    importance = importance&~LOG_PLOT;


    va_list args;
    va_start(args, message);

    const char *label = (importance == LOG_DEBUG) ? "DEBUG" :
                        (importance == LOG_INFO)  ? "INFO"  :
                        (importance == LOG_ERROR) ? "ERROR":
                                                    "CRITICAL"
                                                    ;

    TickType_t ticks = xTaskGetTickCount();
    uint32_t secs = ticks / configTICK_RATE_HZ;
    uint32_t ms   = (ticks % configTICK_RATE_HZ) * (1000UL / configTICK_RATE_HZ);

    // Print prefix first and remember how much was written
    int prefix_len;
    if (plot){
      prefix_len = snprintf(curr_message.data, MAX_MESSAGE_SIZE,
                              "%lu.%03lu, ", secs, ms);
    }
    else if (CM7){
      prefix_len = snprintf(curr_message.data, MAX_MESSAGE_SIZE,
                              "CM7 %lu.%03lus %s: ", secs, ms, label);
    }else{
      prefix_len = snprintf(curr_message.data, MAX_MESSAGE_SIZE,
                              "CM4 %lu.%03lus %s: ", secs, ms, label);
    }
    
    // Format the user message directly into the remaining buffer
    int user_len = vsnprintf(curr_message.data + prefix_len,
                             MAX_MESSAGE_SIZE - prefix_len,
                             message, args);

    va_end(args);

    // Add the final newline (if space permits)
    int total_len = prefix_len + user_len;
    if (total_len < MAX_MESSAGE_SIZE - 2) {
        curr_message.data[total_len++] = '\r';
        curr_message.data[total_len++] = '\n';
    } else if (total_len < MAX_MESSAGE_SIZE - 1) {
        curr_message.data[total_len++] = '\n'; // fallback
    }
    curr_message.data[total_len] = '\0'; // null-terminate just in case
    curr_message.len = total_len;
    QueueHandle_t* q = &queue_error;
    switch (importance)
    {
    case LOG_DEBUG:
        q = &queue_debug;
        break;
      case LOG_INFO:
        q = &queue_info;
        break;
      case LOG_ERROR:
        q = &queue_error;
        break;
      case LOG_CRITICAL:
        HAL_UART_Transmit(huart,(uint8_t*)curr_message.data,curr_message.len,HAL_MAX_DELAY);
        return;
      break;
    }
    if(fromISR){
      BaseType_t xHigherPriorityTaskWoken = pdFALSE;
      xQueueSendFromISR(*q,&curr_message,&xHigherPriorityTaskWoken);
      portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
    else{
      xQueueSend(*q,&curr_message,0);
    }
}
void loggerRAW(int importance,char* message, size_t len) {
    bool fromISR = importance&LOG_FROM_ISR;
    bool CM7 = importance&LOG_CM7;
    bool plot = importance&LOG_PLOT;
    importance = importance&~LOG_FROM_ISR;
    importance = importance&~LOG_CM7;
    importance = importance&~LOG_PLOT;

    memcpy(curr_message.data,message,len);
    curr_message.len = len;
    QueueHandle_t* q = &queue_error;
    switch (importance)
    {
    case LOG_DEBUG:
        q = &queue_debug;
        break;
      case LOG_INFO:
        q = &queue_info;
        break;
      case LOG_ERROR:
        q = &queue_error;
        break;
      case LOG_CRITICAL:
        HAL_UART_Transmit(huart,(uint8_t*)curr_message.data,curr_message.len,HAL_MAX_DELAY);
        return;
      break;
    }
    if(fromISR){
      BaseType_t xHigherPriorityTaskWoken = pdFALSE;
      xQueueSendFromISR(*q,&curr_message,&xHigherPriorityTaskWoken);
      portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
    else{
      xQueueSend(*q,&curr_message,0);
    }
}

void loggerInit(bool debug_en, UART_HandleTypeDef* huart_to_use) {
    debug_enabled = debug_en;
    huart = huart_to_use;

    // Najpierw twórz kolejki
    queue_debug = xQueueCreate(MessageQSize, sizeof(Message_t));
    configASSERT(queue_debug != NULL);
    queue_info = xQueueCreate(MessageQSize, sizeof(Message_t));
    configASSERT(queue_info != NULL);
    queue_error = xQueueCreate(MessageQSize, sizeof(Message_t));
    configASSERT(queue_error != NULL);

    xMutexUART1 = xSemaphoreCreateMutex();
    configASSERT(xMutexUART1 != NULL);

    // Dopiero teraz twórz taski
    BaseType_t status;
    status = xTaskCreate(send_debug, "Send_Debug_Task", 1024, NULL, 2, &task_send_debug);
    configASSERT(status == pdPASS);
    status = xTaskCreate(send_info, "Send_Info_Task", 1024, NULL, 3, &task_send_info);
    configASSERT(status == pdPASS);
    status = xTaskCreate(send_error, "Send_Error_Task", 1024, NULL, 5, &task_send_error);
    configASSERT(status == pdPASS);
}


static void send_debug(void* pvParameters){
  (void)pvParameters;  // Silence the unused parameters (It has to be present for the freeRTOS api)
  //SEGGER_SYSVIEW_PrintfTarget("Debug Send Task");
  while (1)
  {
    if(xQueueReceive(queue_debug, &sending_message_debug, portMAX_DELAY)){
      if(!debug_enabled){
        continue;
      }
      vTaskDelay(pdMS_TO_TICKS(1));
      if(xSemaphoreTake(xMutexUART1,portMAX_DELAY) == pdTRUE)
      {
        task_DMA_UART_Notify = xTaskGetCurrentTaskHandle();
        HAL_UART_Transmit_DMA(huart,(uint8_t*)sending_message_debug.data,sending_message_debug.len);
        // ulTaskNotifyTakeIndexed(NOTIFICATION_DMA,pdTRUE,portMAX_DELAY);
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        xSemaphoreGive(xMutexUART1);
      }
    }
  }
  
}
static void send_info(void* pvParameters){
  (void)pvParameters;  // Silence the unused parameters (It has to be present for the freeRTOS api)
  //SEGGER_SYSVIEW_PrintfTarget("Info Send Task");
  while (1)
  {
    if(xQueueReceive(queue_info, &sending_message_info, portMAX_DELAY)){
      vTaskDelay(pdMS_TO_TICKS(1));
      if(xSemaphoreTake(xMutexUART1,portMAX_DELAY) == pdTRUE)
      {
        task_DMA_UART_Notify = xTaskGetCurrentTaskHandle();
        HAL_UART_Transmit_DMA(huart,(uint8_t*)sending_message_info.data,sending_message_info.len);
        // ulTaskNotifyTakeIndexed(NOTIFICATION_DMA,pdTRUE, portMAX_DELAY);
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        xSemaphoreGive(xMutexUART1);
      }
    }
  }
}
static void send_error(void* pvParameters){
  (void)pvParameters;  // Silence the unused parameters (It has to be present for the freeRTOS api)
  //SEGGER_SYSVIEW_PrintfTarget("Error Send Task");
  while (1)
  {
    if(xQueueReceive(queue_error, &sending_message_error, portMAX_DELAY)){ 
      vTaskDelay(pdMS_TO_TICKS(1)); 
      if(xSemaphoreTake(xMutexUART1,portMAX_DELAY) == pdTRUE)
      {
        task_DMA_UART_Notify = xTaskGetCurrentTaskHandle();
        HAL_UART_Transmit_DMA(huart,(uint8_t*)sending_message_error.data,sending_message_error.len);
        // ulTaskNotifyTakeIndexed(NOTIFICATION_DMA,pdTRUE,portMAX_DELAY);
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        xSemaphoreGive(xMutexUART1);
      }
    }
      
  }
}
