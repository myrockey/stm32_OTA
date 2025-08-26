#ifndef __TIM_H
#define __TIM_H

//WIFI
#define WIFI_TIM_APBX 	    RCC_APB1PeriphClockCmd
#define WIFI_TIM_CLK 	    RCC_APB1Periph_TIM4
#define WIFI_TIM 	        TIM4
#define WIFI_TIM_IRQn       TIM4_IRQn
#define WIFI_TIM_IRQHandler TIM4_IRQHandler

void TIM_WIFI_ENABLE_30S(void);
void TIM_WIFI_ENABLE_2S(void);

#endif

