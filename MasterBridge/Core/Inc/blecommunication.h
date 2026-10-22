/*
 * blecommunication.h
 *
 *  Created on: 22 Oct 2026
 *      Author: Luciano Massaneiro
 */

#ifndef INC_BLECOMMUNICATION_H_
#define INC_BLECOMMUNICATION_H_

#include "main.h"
#include "leds.h"
#include "master.h"

static inline void CheckConnection(){
	if(LED1_GPIO_Port->ODR & LED1_Pin)
		LED1LOW;
	else
		LED1HIGH;
}

#endif /* INC_BLECOMMUNICATION_H_ */
