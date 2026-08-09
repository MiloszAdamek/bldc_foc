/*
 * commander.h
 *
 *  Created on: Nov 11, 2025
 *      Author: Milosz Adamek
 */

#ifndef INC_COMMANDER_H_
#define INC_COMMANDER_H_

#include "BSP/board.h"

void Commander_Init(BoardHandleTypeDef* board);

//Funkcja do cyklicznego wywoływania w pętli głównej.
//Sprawdza, czy nadeszła nowa komenda i ją przetwarza.
void Commander_Process(void);

#endif /* INC_COMMANDER_H_ */
