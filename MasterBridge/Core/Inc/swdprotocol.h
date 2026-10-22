/*
 * swdprotocol.h
 *
 *  Created on: 22 Oct 2026
 *      Author: Luciano Massaneiro
 */

#ifndef INC_SWDPROTOCOL_H_
#define INC_SWDPROTOCOL_H_

#define SWDACKOK 0b001UL
#define SWDACKWAIT 0b010UL
#define SWDACKFAULT 0b100UL

#define SWDREQUESTISREAD 0b100UL

#define SWDLINECOMMAND 0xE79EUL

typedef struct{
	uint8_t request;
	uint8_t ack;
	uint32_t data;
	uint8_t parity;
}SWDPacket;

#endif /* INC_SWDPROTOCOL_H_ */
