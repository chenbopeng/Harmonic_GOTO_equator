#ifndef __DMA_H
#define	__DMA_H

#include "stm32f10x.h"
#include "usart.h"	


void DMA_RX_init(u32 buffer,u16 size);
void DMA_TX_init(u32 buffer,u16 size);
void DMA_SEND_DATA(u32 buffer, u16 size);


#endif /* __COMMAND_H */

