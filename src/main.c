#define __MAIN_C__

//
// includes
//

#include "stm32f4xx_conf.h"
#include "stm32f4_discovery.h"
#include "usbd_hid_core.h"
#include "usbd_usr.h"
#include "usbd_desc.h"

//
// global variables
//

__ALIGN_BEGIN USB_OTG_CORE_HANDLE  USB_OTG_dev __ALIGN_END;

//
// functions
//

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
    { }
}

void GPIO_init(void)
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
}

static uint8_t HID_OutData(void* pdev, uint8_t epnum, uint8_t* buf, uint16_t len)
{
    /* Echo buffer back */
    USBD_HID_SendReport(pdev, buf, len);

    GPIO_init();
    /* Reset LEDs */
    GPIO_ResetBits(GPIOD, GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15);
    /* Set pins to be toggled */
    if (*buf == 'a')
        GPIO_SetBits(GPIOD, GPIO_Pin_12);
    if (*buf == 'b')
        GPIO_SetBits(GPIOD, GPIO_Pin_13);
    if (*buf == 'c')
        GPIO_SetBits(GPIOD, GPIO_Pin_14);
    if (*buf == 'd')
        GPIO_SetBits(GPIOD, GPIO_Pin_15);
}

void USB_init(void)
{
    USBD_Init(&USB_OTG_dev,
            USB_OTG_FS_CORE_ID,
            &USR_desc,
            &USBD_HID_cb,
            &USR_cb);
    USBD_HID_SetRecieveReportCB(&USB_OTG_dev, HID_OutData);
}

void usb_app(void)
{
    // init USB
    USB_init();
    // mainloop (do nothing)
    while (1);
    // disconnect the USB device
    DCD_DevDisconnect(&USB_OTG_dev);
    USB_OTG_StopDevice(&USB_OTG_dev);
}

void led_app(void)
{
    // init GPIO
    GPIO_init();
    // mainloop (flash LEDs)
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
        /* Reset LEDs */
        GPIO_ResetBits(GPIOD, GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15);
        /* Insert delay */
        Delay(0xFFFFFF);
    }
}

int main (void)
{
    usb_app();
    //led_app();
    return 0;
}
