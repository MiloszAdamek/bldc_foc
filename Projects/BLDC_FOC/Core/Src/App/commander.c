/*
 * commander.c
 *
 *  Created on: Nov 11, 2025
 *      Author: Miloush
 */

#include "App/commander.h"
#include "App/motor_control.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "usart.h"

static UART_HandleTypeDef* cmd_huart; // Przechowuje wskaźnik do używanego UART

#define UART_RX_BUFFER_SIZE 64
static uint8_t g_uart_rx_buffer[UART_RX_BUFFER_SIZE];
static uint8_t g_uart_rx_char;
static volatile uint8_t g_uart_rx_index = 0;
static volatile bool g_new_command_flag = false;

static void process_command(char* cmd);

// --- Implementacja publicznego API ---

void Commander_Init(void* huart_void) {
    if (huart_void == NULL) {
        Error_Handler();
    }
    cmd_huart = (UART_HandleTypeDef*)huart_void;

    HAL_UART_Receive_IT(cmd_huart, &g_uart_rx_char, 1);

    printf("\r\n--- Commander ---\r\n");
    printf("Commands:\r\n");
    printf(" X               		(Start Motor / Closed Loop)\r\n");
    printf(" B               		(Stop Motor / Idle)\r\n");
    printf(" V <velocity>    		(RPM)\r\n");
    printf(" T <torque>      		(Amps)\r\n");
    printf(" R               		(Reboot Slave)\r\n");
    printf("> ");
}

void Commander_Process(void) {
    if (g_new_command_flag) {
        g_new_command_flag = false;

        g_uart_rx_buffer[g_uart_rx_index] = '\0';
        process_command((char*)g_uart_rx_buffer);

        g_uart_rx_index = 0;
        memset(g_uart_rx_buffer, 0, UART_RX_BUFFER_SIZE);

        printf("> ");
    }
}

// --- Funkcje prywatne (static) ---

static void process_command(char* cmd) {
    char* command_token = strtok(cmd, " ");

    if (command_token == NULL) {
        return;
    }

    char command_char = command_token[0];
	char* args = strtok(NULL, ""); // Pobierz resztę stringa jako argumenty
	switch(command_char){
		case 'V':
		case 'v': {
			float velocity = atof(args);
			printf("CMD: Set Velocity -> %.2f RPM\r\n", velocity);
			MotorControl_SetSpeed(velocity);
			break;
		}
		case 'T':
		case 't': {
			float torque = atof(args);
			printf("CMD: Set Torque -> %.2f A\r\n", torque);
			MotorControl_SetTorque(torque);
			break;
		}
		case 'R':
		case 'r': {
			printf("CMD: Reboot\r\n");
			MotorControl_Reboot();
			break;
		}
		case 'X': // Start Motor
		case 'x': {
			printf("CMD: Start\r\n");
			MotorControl_Start();
			break;
		}
		case 'B': // Stop Motor
		case 'b': {
			printf("CMD: Stop\r\n");
			MotorControl_Stop();
			break;
		}
		default:
			printf("Error: Unknown command '%c'\r\n", command_char);
			break;
	 }
}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == cmd_huart->Instance) {
        if (g_uart_rx_char == '\r' || g_uart_rx_char == '\n') {
            printf("\r\n");
            if (g_uart_rx_index > 0) {
                g_new_command_flag = true;
            } else {
                printf("> ");
            }
        }
        else if (g_uart_rx_char == '\b' || g_uart_rx_char == 127) {
            if (g_uart_rx_index > 0) {
                g_uart_rx_index--;
                printf("\b \b");
            }
        }
        else if (g_uart_rx_index < UART_RX_BUFFER_SIZE - 1) {
            g_uart_rx_buffer[g_uart_rx_index++] = g_uart_rx_char;
            printf("%c", g_uart_rx_char);
        }

        // Ponownie włącz nasłuchiwanie
        HAL_UART_Receive_IT(cmd_huart, &g_uart_rx_char, 1);
    }
}
