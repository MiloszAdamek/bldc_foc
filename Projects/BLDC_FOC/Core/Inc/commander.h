/*
 * commander.h
 *
 *  Created on: Nov 11, 2025
 *      Author: Miloush
 */

#ifndef INC_COMMANDER_H_
#define INC_COMMANDER_H_

/**
 * @brief Inicjalizuje moduł Commandera.
 * @param huart_void Wskaźnik na uchwyt UART_HandleTypeDef (np. &huart2).
 */
void Commander_Init(void* huart_void);

/**
 * @brief Funkcja do cyklicznego wywoływania w pętli głównej.
 *        Sprawdza, czy nadeszła nowa komenda i ją przetwarza.
 */
void Commander_Process(void);

#endif /* INC_COMMANDER_H_ */
