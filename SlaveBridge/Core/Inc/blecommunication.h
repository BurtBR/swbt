/*
 * communication.h
 *
 *  Created on: 21 Oct 2026
 *      Author: Luciano Massaneiro
 */

#ifndef INC_BLECOMMUNICATION_H_
#define INC_BLECOMMUNICATION_H_

#include "main.h"
#include "leds.h"
#include "swdprotocol.h"

typedef struct{
	uint8_t ready, busy;
	SWDPacket packet;
}BLEStatus;

volatile extern BLEStatus blestatus;

static inline void SendLineReset(){

}

static inline void SendLineSWD(){

}

static inline void SendPacket(const SWDPacket *packet){

}

static inline void CheckConnection(){
	if(LED1_GPIO_Port->ODR & LED1_Pin)
		LED1LOW;
	else
		LED1HIGH;
}

#endif /* INC_BLECOMMUNICATION_H_ */
