#define __MAIN_C__

//
// includes
//

#include "stm32f4xx_conf.h"
#include "stm32f4_discovery.h"
#include "usbd_hid_core.h"
#include "usbd_usr.h"
#include "usbd_desc.h"
#include "prot.h"

//
// global variables
//

__ALIGN_BEGIN USB_OTG_CORE_HANDLE  USB_OTG_dev __ALIGN_END;

#define HID_OUT_FIFO_COUNT 10
char HID_out_fifo[HID_OUT_FIFO_COUNT][HID_OUT_PACKET];
int HID_out_write_index = 0;
int HID_out_read_index = 0;

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

static void HID_OutData(USB_OTG_CORE_HANDLE* pdev, uint8_t epnum, uint8_t* buf, uint16_t len)
{
    prot_process_packet(buf, len);
}

static void HID_SendReport(void* buffer, size_t buffer_size)
{
    //TODO: THIS SHOULD BE IN A CRITICAL SECTION!!!
    // copy report to HID_out_fifo
    if (HID_out_write_index < HID_OUT_FIFO_COUNT-1)
        HID_out_write_index++;
    else
        HID_out_write_index = 0;
    memcpy(HID_out_fifo[HID_out_write_index], buffer, MIN(buffer_size, HID_OUT_PACKET));
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

void SysTick_Handler(void)
{
    // Send report from HID_out_fifo
    if (HID_out_read_index != HID_out_write_index)
    {
        if (HID_out_read_index < HID_out_write_index)
            HID_out_read_index++;
        else
        {
            if (HID_out_read_index < HID_OUT_FIFO_COUNT-1)
                HID_out_read_index++;
            else
                HID_out_read_index = 0;
        }
        USBD_HID_SendReport(&USB_OTG_dev, HID_out_fifo[HID_out_read_index], HID_OUT_PACKET);
    }
}

void usb_app(void)
{
    /* 
     * Setup SysTick Timer for 1 msec interrupts.
     */
    if (SysTick_Config(SystemCoreClock / 1000))
    {
        while (1);                  /* Capture error */
    }
    
    // init USB
    USB_init();
    // init prot
    prot_init(HID_SendReport);
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
