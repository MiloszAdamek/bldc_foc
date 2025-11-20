/*
 * commander.h
 *
 *  Created on: Nov 11, 2025
 *      Author: Miloush
 */

#ifndef INC_COMMANDER_H_
#define INC_COMMANDER_H_

void Commander_Init(void* huart_void);

//Funkcja do cyklicznego wywoływania w pętli głównej.
//Sprawdza, czy nadeszła nowa komenda i ją przetwarza.
void Commander_Process(void);

#endif /* INC_COMMANDER_H_ */
