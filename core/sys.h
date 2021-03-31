#ifndef SYS_H
#define SYS_H

#include "main.h"

void CORE_Sys_Init(TIM_HandleTypeDef* tick, IRQn_Type irq);

void CORE_Sys_Run(void);

#endif
