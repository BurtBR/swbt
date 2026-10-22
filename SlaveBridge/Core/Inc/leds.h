/*
 * leds.h
 *
 *  Created on: 21 Oct 2026
 *      Author: Luciano Massaneiro
 */

#ifndef INC_LEDS_H_
#define INC_LEDS_H_

#include "main.h"

#define LED1HIGH LED1_GPIO_Port->BSRR = LED1_Pin
#define LED1LOW LED1_GPIO_Port->BSRR = (LED1_Pin << 16UL)

#define LED2HIGH LED2_GPIO_Port->BSRR = LED2_Pin
#define LED2LOW LED2_GPIO_Port->BSRR = (LED2_Pin << 16UL)

#define LED3HIGH LED3_GPIO_Port->BSRR = LED3_Pin
#define LED3LOW LED3_GPIO_Port->BSRR = (LED3_Pin << 16UL)

#endif /* INC_LEDS_H_ */
