#ifndef __LED_H
#define __LED_H

#define LED1_GPIO_PIN GPIO_Pin_4
#define LED2_GPIO_PIN GPIO_Pin_5

void LED_Init(void);
void LED1_ON(void);
void LED1_OFF(void);
void LED1_Turn(void);
void LED2_ON(void);
void LED2_OFF(void);
void LED2_Turn(void);

#endif
