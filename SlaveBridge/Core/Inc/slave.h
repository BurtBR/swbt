/*
 * slave.h
 *
 *  Created on: 21 Oct 2026
 *      Author: Luciano Massaneiro
 */

#ifndef INC_SLAVE_H_
#define INC_SLAVE_H_

#include "blecommunication.h"
#include "main.h"
#include "swdprotocol.h"

#define SLAVEDMASIZE 256U

#define SWDIOINSHIFT __builtin_ctz(SWDIOIN_Pin)
#define SWDIOINHIGH SWDIOIN_Pin
#define SWDIOINLOW (SWDIOIN_Pin << 16UL)
#define SWDIOINDATA(bitdata) ( SWDIOIN_Pin << ((((bitdata) ^ 1UL)) << 4UL) )
#define SWDIOINMODERMASK (0x3 << (2UL*SWDIOINSHIFT))
#define SWDIOINDRIVE ((SWDIOIN_GPIO_Port->MODER & (~SWDIOINMODERMASK)) | (0x1 << (2UL*SWDIOINSHIFT)))
#define SWDIOINHIZ (SWDIOIN_GPIO_Port->MODER & (~SWDIOINMODERMASK))

typedef enum{
	LineReset0,
	LineReset1,
	LineSWD,
	Start,
	Write,
	WriteParity,
	Read,
}SlaveState;

extern DMA_HandleTypeDef hdma_tim2_ch1;
extern DMA_HandleTypeDef hdma_tim2_ch2;
extern DMA_HandleTypeDef hdma_tim2_ch3;
extern volatile uint32_t DMARxBuffer[];
extern volatile uint32_t DMATxBuffer[];
extern volatile uint32_t DMAMODERBuffer[];

static inline uint32_t SlaveDMARxIndex(){
    return (SLAVEDMASIZE - DMA1_Channel1->CNDTR) & (SLAVEDMASIZE - 1U);
}

static inline uint8_t SlaveDMABit(const uint8_t dmaidx){
	return (DMARxBuffer[dmaidx] >> SWDIOINSHIFT) & 1UL;
}

static inline void SlaveSetDMAAckOk(const uint8_t dmaackidx){
	const uint32_t data = SWDIOINDRIVE;
	DMAMODERBuffer[(dmaackidx-1U)&0xFF] = data;
	DMAMODERBuffer[dmaackidx] = data;
	DMAMODERBuffer[(dmaackidx+1U)&0xFF] = data;

	DMATxBuffer[(dmaackidx-1U)&0xFF] = SWDIOINHIGH;
	DMATxBuffer[dmaackidx] = SWDIOINLOW;
	DMATxBuffer[(dmaackidx+1U)&0xFF] = SWDIOINLOW;
}

static inline void SlaveSetDMAAckWait(const uint8_t dmaackidx){
	const uint32_t data = SWDIOINDRIVE;
	DMAMODERBuffer[(dmaackidx-1U)&0xFF] = data;
	DMAMODERBuffer[dmaackidx] = data;
	DMAMODERBuffer[(dmaackidx+1U)&0xFF] = data;
	DMAMODERBuffer[(dmaackidx+2U)&0xFF] = SWDIOINHIZ;

	DMATxBuffer[(dmaackidx-1U)&0xFF] = SWDIOINLOW;
	DMATxBuffer[dmaackidx] = SWDIOINHIGH;
	DMATxBuffer[(dmaackidx+1U)&0xFF] = SWDIOINLOW;
	DMATxBuffer[(dmaackidx+2U)&0xFF] = 0x0;
}

static inline void SlaveSetDMAAckFault(const uint8_t dmaackidx){
	const uint32_t data = SWDIOINDRIVE;
	DMAMODERBuffer[(dmaackidx-1U)&0xFF] = data;
	DMAMODERBuffer[dmaackidx] = data;
	DMAMODERBuffer[(dmaackidx+1U)&0xFF] = data;
	DMAMODERBuffer[(dmaackidx+2U)&0xFF] = SWDIOINHIZ;

	DMATxBuffer[(dmaackidx-1U)&0xFF] = SWDIOINLOW;
	DMATxBuffer[dmaackidx] = SWDIOINLOW;
	DMATxBuffer[(dmaackidx+1U)&0xFF] = SWDIOINHIGH;
	DMATxBuffer[(dmaackidx+2U)&0xFF] = 0x0;
}

static inline void SlaveSetDMARequest(const uint8_t dmaidx){
	const uint32_t data = SWDIOINHIZ;
	DMAMODERBuffer[(dmaidx-1U)&0xFF] = data;
	DMAMODERBuffer[dmaidx] = data;
	DMAMODERBuffer[(dmaidx+1U)&0xFF] = data;
	DMAMODERBuffer[(dmaidx+2U)&0xFF] = data;
	DMAMODERBuffer[(dmaidx+3U)&0xFF] = data;
	DMAMODERBuffer[(dmaidx+4U)&0xFF] = data;
	DMAMODERBuffer[(dmaidx+5U)&0xFF] = data;
	DMAMODERBuffer[(dmaidx+6U)&0xFF] = data;
	DMAMODERBuffer[(dmaidx+7U)&0xFF] = data;

	DMATxBuffer[(dmaidx-1U)&0xFF] = 0x0;
	DMATxBuffer[dmaidx] = 0x0;
	DMATxBuffer[(dmaidx+1U)&0xFF] = 0x0;
	DMATxBuffer[(dmaidx+2U)&0xFF] = 0x0;
	DMATxBuffer[(dmaidx+3U)&0xFF] = 0x0;
	DMATxBuffer[(dmaidx+4U)&0xFF] = 0x0;
	DMATxBuffer[(dmaidx+5U)&0xFF] = 0x0;
	DMATxBuffer[(dmaidx+6U)&0xFF] = 0x0;
	DMATxBuffer[(dmaidx+7U)&0xFF] = 0x0;
}

static inline void SlaveSet2DMARequest(const uint8_t dmaidx){
	SlaveSetDMARequest(dmaidx);
	SlaveSetDMAAckWait(dmaidx+9);
	SlaveSetDMARequest(dmaidx+13);
}

static inline void SlaveSetDMARead(const uint8_t dmareadidx, uint32_t *data){
	const uint32_t moderegister = SWDIOINDRIVE;
	DMATxBuffer[(dmareadidx-1)&0xFF] = SWDIOINDATA((*data) & 0x1UL);
	DMATxBuffer[dmareadidx] = SWDIOINDATA(((*data) >> 1UL) & 0x1UL);
	DMATxBuffer[(dmareadidx+1)&0xFF] = SWDIOINDATA(((*data) >> 2UL) & 0x1UL);
	DMATxBuffer[(dmareadidx+2)&0xFF] = SWDIOINDATA(((*data) >> 3UL) & 0x1UL);
	DMATxBuffer[(dmareadidx+3)&0xFF] = SWDIOINDATA(((*data) >> 4UL) & 0x1UL);
	DMATxBuffer[(dmareadidx+4)&0xFF] = SWDIOINDATA(((*data) >> 5UL) & 0x1UL);
	DMATxBuffer[(dmareadidx+5)&0xFF] = SWDIOINDATA(((*data) >> 6UL) & 0x1UL);
	DMATxBuffer[(dmareadidx+6)&0xFF] = SWDIOINDATA(((*data) >> 7UL) & 0x1UL);
	DMATxBuffer[(dmareadidx+7)&0xFF] = SWDIOINDATA(((*data) >> 8UL) & 0x1UL);
	DMATxBuffer[(dmareadidx+8)&0xFF] = SWDIOINDATA(((*data) >> 9UL) & 0x1UL);
	DMATxBuffer[(dmareadidx+9)&0xFF] = SWDIOINDATA(((*data) >> 10UL) & 0x1UL);
	DMATxBuffer[(dmareadidx+10)&0xFF] = SWDIOINDATA(((*data) >> 11UL) & 0x1UL);
	DMATxBuffer[(dmareadidx+11)&0xFF] = SWDIOINDATA(((*data) >> 12UL) & 0x1UL);
	DMATxBuffer[(dmareadidx+12)&0xFF] = SWDIOINDATA(((*data) >> 13UL) & 0x1UL);
	DMATxBuffer[(dmareadidx+13)&0xFF] = SWDIOINDATA(((*data) >> 14UL) & 0x1UL);
	DMATxBuffer[(dmareadidx+14)&0xFF] = SWDIOINDATA(((*data) >> 15UL) & 0x1UL);
	DMATxBuffer[(dmareadidx+15)&0xFF] = SWDIOINDATA(((*data) >> 16UL) & 0x1UL);
	DMATxBuffer[(dmareadidx+16)&0xFF] = SWDIOINDATA(((*data) >> 17UL) & 0x1UL);
	DMATxBuffer[(dmareadidx+17)&0xFF] = SWDIOINDATA(((*data) >> 18UL) & 0x1UL);
	DMATxBuffer[(dmareadidx+18)&0xFF] = SWDIOINDATA(((*data) >> 19UL) & 0x1UL);
	DMATxBuffer[(dmareadidx+19)&0xFF] = SWDIOINDATA(((*data) >> 20UL) & 0x1UL);
	DMATxBuffer[(dmareadidx+20)&0xFF] = SWDIOINDATA(((*data) >> 21UL) & 0x1UL);
	DMATxBuffer[(dmareadidx+21)&0xFF] = SWDIOINDATA(((*data) >> 22UL) & 0x1UL);
	DMATxBuffer[(dmareadidx+22)&0xFF] = SWDIOINDATA(((*data) >> 23UL) & 0x1UL);
	DMATxBuffer[(dmareadidx+23)&0xFF] = SWDIOINDATA(((*data) >> 24UL) & 0x1UL);
	DMATxBuffer[(dmareadidx+24)&0xFF] = SWDIOINDATA(((*data) >> 25UL) & 0x1UL);
	DMATxBuffer[(dmareadidx+25)&0xFF] = SWDIOINDATA(((*data) >> 26UL) & 0x1UL);
	DMATxBuffer[(dmareadidx+26)&0xFF] = SWDIOINDATA(((*data) >> 27UL) & 0x1UL);
	DMATxBuffer[(dmareadidx+27)&0xFF] = SWDIOINDATA(((*data) >> 28UL) & 0x1UL);
	DMATxBuffer[(dmareadidx+28)&0xFF] = SWDIOINDATA(((*data) >> 29UL) & 0x1UL);
	DMATxBuffer[(dmareadidx+29)&0xFF] = SWDIOINDATA(((*data) >> 30UL) & 0x1UL);
	DMATxBuffer[(dmareadidx+30)&0xFF] = SWDIOINDATA(((*data) >> 31UL) & 0x1UL);
	DMATxBuffer[(dmareadidx+31)&0xFF] = SWDIOINDATA(__builtin_parity(*data));
	DMATxBuffer[(dmareadidx+32)&0xFF] = 0x0;

	DMAMODERBuffer[(dmareadidx-1)&0xFF] = moderegister;
	DMAMODERBuffer[dmareadidx] = moderegister;
	DMAMODERBuffer[(dmareadidx+1)&0xFF] = moderegister;
	DMAMODERBuffer[(dmareadidx+2)&0xFF] = moderegister;
	DMAMODERBuffer[(dmareadidx+3)&0xFF] = moderegister;
	DMAMODERBuffer[(dmareadidx+4)&0xFF] = moderegister;
	DMAMODERBuffer[(dmareadidx+5)&0xFF] = moderegister;
	DMAMODERBuffer[(dmareadidx+6)&0xFF] = moderegister;
	DMAMODERBuffer[(dmareadidx+7)&0xFF] = moderegister;
	DMAMODERBuffer[(dmareadidx+8)&0xFF] = moderegister;
	DMAMODERBuffer[(dmareadidx+9)&0xFF] = moderegister;
	DMAMODERBuffer[(dmareadidx+10)&0xFF] = moderegister;
	DMAMODERBuffer[(dmareadidx+11)&0xFF] = moderegister;
	DMAMODERBuffer[(dmareadidx+12)&0xFF] = moderegister;
	DMAMODERBuffer[(dmareadidx+13)&0xFF] = moderegister;
	DMAMODERBuffer[(dmareadidx+14)&0xFF] = moderegister;
	DMAMODERBuffer[(dmareadidx+15)&0xFF] = moderegister;
	DMAMODERBuffer[(dmareadidx+16)&0xFF] = moderegister;
	DMAMODERBuffer[(dmareadidx+17)&0xFF] = moderegister;
	DMAMODERBuffer[(dmareadidx+18)&0xFF] = moderegister;
	DMAMODERBuffer[(dmareadidx+19)&0xFF] = moderegister;
	DMAMODERBuffer[(dmareadidx+20)&0xFF] = moderegister;
	DMAMODERBuffer[(dmareadidx+21)&0xFF] = moderegister;
	DMAMODERBuffer[(dmareadidx+22)&0xFF] = moderegister;
	DMAMODERBuffer[(dmareadidx+23)&0xFF] = moderegister;
	DMAMODERBuffer[(dmareadidx+24)&0xFF] = moderegister;
	DMAMODERBuffer[(dmareadidx+25)&0xFF] = moderegister;
	DMAMODERBuffer[(dmareadidx+26)&0xFF] = moderegister;
	DMAMODERBuffer[(dmareadidx+27)&0xFF] = moderegister;
	DMAMODERBuffer[(dmareadidx+28)&0xFF] = moderegister;
	DMAMODERBuffer[(dmareadidx+29)&0xFF] = moderegister;
	DMAMODERBuffer[(dmareadidx+30)&0xFF] = moderegister;
	DMAMODERBuffer[(dmareadidx+31)&0xFF] = moderegister;
	DMAMODERBuffer[(dmareadidx+32)&0xFF] = SWDIOINHIZ;
}

static inline void SlaveSetDMAWrite(const uint8_t dmawriteidx){
	const uint32_t data = SWDIOINHIZ;
	DMAMODERBuffer[(dmawriteidx-1)&0xFF] = data;
	DMAMODERBuffer[dmawriteidx] = data;
	DMAMODERBuffer[(dmawriteidx+1)&0xFF] = data;
	DMAMODERBuffer[(dmawriteidx+2)&0xFF] = data;
	DMAMODERBuffer[(dmawriteidx+3)&0xFF] = data;
	DMAMODERBuffer[(dmawriteidx+4)&0xFF] = data;
	DMAMODERBuffer[(dmawriteidx+5)&0xFF] = data;
	DMAMODERBuffer[(dmawriteidx+6)&0xFF] = data;
	DMAMODERBuffer[(dmawriteidx+7)&0xFF] = data;
	DMAMODERBuffer[(dmawriteidx+8)&0xFF] = data;
	DMAMODERBuffer[(dmawriteidx+9)&0xFF] = data;
	DMAMODERBuffer[(dmawriteidx+10)&0xFF] = data;
	DMAMODERBuffer[(dmawriteidx+11)&0xFF] = data;
	DMAMODERBuffer[(dmawriteidx+12)&0xFF] = data;
	DMAMODERBuffer[(dmawriteidx+13)&0xFF] = data;
	DMAMODERBuffer[(dmawriteidx+14)&0xFF] = data;
	DMAMODERBuffer[(dmawriteidx+15)&0xFF] = data;
	DMAMODERBuffer[(dmawriteidx+16)&0xFF] = data;
	DMAMODERBuffer[(dmawriteidx+17)&0xFF] = data;
	DMAMODERBuffer[(dmawriteidx+18)&0xFF] = data;
	DMAMODERBuffer[(dmawriteidx+19)&0xFF] = data;
	DMAMODERBuffer[(dmawriteidx+20)&0xFF] = data;
	DMAMODERBuffer[(dmawriteidx+21)&0xFF] = data;
	DMAMODERBuffer[(dmawriteidx+22)&0xFF] = data;
	DMAMODERBuffer[(dmawriteidx+23)&0xFF] = data;
	DMAMODERBuffer[(dmawriteidx+24)&0xFF] = data;
	DMAMODERBuffer[(dmawriteidx+25)&0xFF] = data;
	DMAMODERBuffer[(dmawriteidx+26)&0xFF] = data;
	DMAMODERBuffer[(dmawriteidx+27)&0xFF] = data;
	DMAMODERBuffer[(dmawriteidx+28)&0xFF] = data;
	DMAMODERBuffer[(dmawriteidx+29)&0xFF] = data;
	DMAMODERBuffer[(dmawriteidx+30)&0xFF] = data;
	DMAMODERBuffer[(dmawriteidx+31)&0xFF] = data;
	DMAMODERBuffer[(dmawriteidx+32)&0xFF] = data;

	DMATxBuffer[(dmawriteidx-1)&0xFF] = 0x0;
	DMATxBuffer[dmawriteidx] = 0x0;
	DMATxBuffer[(dmawriteidx+1)&0xFF] = 0x0;
	DMATxBuffer[(dmawriteidx+2)&0xFF] = 0x0;
	DMATxBuffer[(dmawriteidx+3)&0xFF] = 0x0;
	DMATxBuffer[(dmawriteidx+4)&0xFF] = 0x0;
	DMATxBuffer[(dmawriteidx+5)&0xFF] = 0x0;
	DMATxBuffer[(dmawriteidx+6)&0xFF] = 0x0;
	DMATxBuffer[(dmawriteidx+7)&0xFF] = 0x0;
	DMATxBuffer[(dmawriteidx+8)&0xFF] = 0x0;
	DMATxBuffer[(dmawriteidx+9)&0xFF] = 0x0;
	DMATxBuffer[(dmawriteidx+10)&0xFF] = 0x0;
	DMATxBuffer[(dmawriteidx+11)&0xFF] = 0x0;
	DMATxBuffer[(dmawriteidx+12)&0xFF] = 0x0;
	DMATxBuffer[(dmawriteidx+13)&0xFF] = 0x0;
	DMATxBuffer[(dmawriteidx+14)&0xFF] = 0x0;
	DMATxBuffer[(dmawriteidx+15)&0xFF] = 0x0;
	DMATxBuffer[(dmawriteidx+16)&0xFF] = 0x0;
	DMATxBuffer[(dmawriteidx+17)&0xFF] = 0x0;
	DMATxBuffer[(dmawriteidx+18)&0xFF] = 0x0;
	DMATxBuffer[(dmawriteidx+19)&0xFF] = 0x0;
	DMATxBuffer[(dmawriteidx+20)&0xFF] = 0x0;
	DMATxBuffer[(dmawriteidx+21)&0xFF] = 0x0;
	DMATxBuffer[(dmawriteidx+22)&0xFF] = 0x0;
	DMATxBuffer[(dmawriteidx+23)&0xFF] = 0x0;
	DMATxBuffer[(dmawriteidx+24)&0xFF] = 0x0;
	DMATxBuffer[(dmawriteidx+25)&0xFF] = 0x0;
	DMATxBuffer[(dmawriteidx+26)&0xFF] = 0x0;
	DMATxBuffer[(dmawriteidx+27)&0xFF] = 0x0;
	DMATxBuffer[(dmawriteidx+28)&0xFF] = 0x0;
	DMATxBuffer[(dmawriteidx+29)&0xFF] = 0x0;
	DMATxBuffer[(dmawriteidx+30)&0xFF] = 0x0;
	DMATxBuffer[(dmawriteidx+31)&0xFF] = 0x0;
	DMATxBuffer[(dmawriteidx+32)&0xFF] = 0x0;
}

static inline void SlaveSetDMADiscard(const uint8_t dmaidx){
	DMAMODERBuffer[dmaidx] = SWDIOINHIZ;
	DMATxBuffer[dmaidx] = 0x0;
}

static inline void SlaveClearAllDMA(){
	for(int k=0; k<256 ;k++){
		DMAMODERBuffer[k] = SWDIOINHIZ;
		DMATxBuffer[k] = 0x0;
	}
}

static inline void StartDMA(){
	// ----------- CHANNEL 1 -----------
	// Disable TIM2 Channel 1 DMA requests
	TIM2->DIER &= ~TIM_DIER_CC1DE;
	// Start DMA: GPIOA->IDR to circular buffer
	if (HAL_DMA_Start(
			&hdma_tim2_ch1,
			(uint32_t)&GPIOA->IDR,
			(uint32_t)DMARxBuffer,
			SLAVEDMASIZE) != HAL_OK
	){
		Error_Handler();
	}
	// Clear pending capture flags
	TIM2->SR = ~(TIM_SR_CC1IF | TIM_SR_CC1OF);
	// Enable DMA requests
	TIM2->DIER |= TIM_DIER_CC1DE;
	// ----------- CHANNEL 1 -----------

	// ----------- CHANNEL 2 -----------
	// Disable Channel 2 DMA requests
	TIM2->DIER &= ~TIM_DIER_CC2DE;
	// Memory -> GPIOA BSRR
	if (HAL_DMA_Start_IT(
		&hdma_tim2_ch2,
		(uint32_t)DMATxBuffer,
		(uint32_t)&GPIOA->BSRR,
		SLAVEDMASIZE) != HAL_OK
	){
		Error_Handler();
	}
	// Clear stale Channel 2 capture flag
	TIM2->SR = ~TIM_SR_CC2IF;
	// Enable DMA request on Channel 2 falling edges
	TIM2->DIER |= TIM_DIER_CC2DE;
	// ----------- CHANNEL 2 -----------

	// ----------- CHANNEL 3 -----------
	// Disable TIM2 Channel 3 DMA requests
	TIM2->DIER &= ~TIM_DIER_CC3DE;
	// Start DMA: GPIOA->IDR to circular buffer
	if (HAL_DMA_Start(
		&hdma_tim2_ch3,
		(uint32_t)DMAMODERBuffer,
		(uint32_t)&GPIOA->MODER,
		SLAVEDMASIZE) != HAL_OK
	){
		Error_Handler();
	}
	// Clear pending capture flags
	TIM2->SR = ~(TIM_SR_CC3IF | TIM_SR_CC3OF);
	// Enable DMA requests
	TIM2->DIER |= TIM_DIER_CC3DE;
	// ----------- CHANNEL 3 -----------
}

#endif /* INC_SLAVE_H_ */
