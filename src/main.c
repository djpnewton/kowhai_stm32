#define __MAIN_C__

#include "stm32f4xx_conf.h"
#include "stm32f4_discovery.h"
#include <stdint.h>

#ifdef  USE_FULL_ASSERT
void assert_failed(uint8_t* file, uint32_t line)
{
  /* Infinite loop */
  while (1)
  {
  }
}
#endif

void Delay(__IO uint32_t nCount)
{
  while(nCount--)
  {
  }
}

int main (void)
{
    GPIO_InitTypeDef GPIO_InitStructure;       
    GPIO_StructInit(&GPIO_InitStructure); 

    /* GPIOD Periph clock enable */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

    /* Configure PD12, PD13, PD14 and PD15 in output pushpull mode */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13| GPIO_Pin_14| GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    while (1)
    {
        /* PD12 to be toggled */
        GPIO_SetBits(GPIOD, GPIO_Pin_12);

        /* Insert delay */
        Delay(0x3FFFFF);

        /* PD13 to be toggled */
        GPIO_SetBits(GPIOD, GPIO_Pin_13);

        /* Insert delay */
        Delay(0x3FFFFF);

        /* PD14 to be toggled */
        GPIO_SetBits(GPIOD, GPIO_Pin_14);

        /* Insert delay */
        Delay(0x3FFFFF);

        /* PD15 to be toggled */
        GPIO_SetBits(GPIOD, GPIO_Pin_15);

        /* Insert delay */
        Delay(0x7FFFFF);

        GPIO_ResetBits(GPIOD, GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15);

        /* Insert delay */
        Delay(0xFFFFFF);
    }

    return 0;
}
