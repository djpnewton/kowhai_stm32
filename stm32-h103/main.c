#include <stdlib.h>
#include <libopencm3/stm32/f1/rcc.h>
#include <libopencm3/stm32/f1/gpio.h>
#include "nrf24l01.h"
#include "usb.h"

#define PTX

#define CMD_DATA 0
#define CMD_ACKWAIT 1
struct ag_payload_t
{
    uint8_t cmd;
    uint8_t data;
};

void _nrf_init(void)
{
    int i;
    int mode;
    // http://gpio.kaltpost.de/?page_id=726
    static nrf_reg_buf addr;
    nrf_init();
    addr.data[0] = 1;
    addr.data[1] = 2;
    addr.data[2] = 3;
    addr.data[3] = 4;
    addr.data[4] = 5;
#ifdef PTX
    mode = NRF_MODE_PTX;
#else
    mode = NRF_MODE_PRX;
#endif
    nrf_preset_esbpl(mode, 40, sizeof(struct ag_payload_t), 3, NRF_RT_DELAY_250, &addr);

    // wait for radio to power up
    for (i = 0; i < 0x400000; i++)
        __asm__("nop");
}

void _nrf_poll(void)
{
#ifndef PTX
    int res;
    nrf_payload p;
    p.size = sizeof(struct ag_payload_t);
    res = nrf_receive_blocking(&p);
    if (res == sizeof(struct ag_payload_t))
    {
        struct ag_payload_t* ag_payload = p.data;
        // echo back data packets via nrf ack
        if (ag_payload->cmd == CMD_DATA)
        {
            nrf_write_ack_pl(&p, 0);
            gpio_toggle(GPIOC, GPIO12);
        }
    }
#endif
}

void serial_read_cb(char c)
{
    int res;
    nrf_payload p;
    struct ag_payload_t* ag_payload = (struct ag_payload_t*)p.data;
    p.size = sizeof(struct ag_payload_t);
    ag_payload->cmd = CMD_DATA;
    ag_payload->data = c;
    res = nrf_send_blocking(&p);
    if (res == sizeof(struct ag_payload_t))
    {
        ag_payload->cmd = CMD_ACKWAIT;
        res = nrf_send_blocking(&p);        
        if (res == sizeof(struct ag_payload_t))
        {
            res = nrf_read_ack_pl(&p);
            if (res == sizeof(struct ag_payload_t))
            {
                usb_write_serial(ag_payload->data);
                gpio_toggle(GPIOC, GPIO12);
            }
        }
    }
}

int main(void)
{
    int i;

    rcc_clock_setup_in_hsi_out_48mhz();

    rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_IOPCEN);

    // led on
    gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_2_MHZ,
        GPIO_CNF_OUTPUT_PUSHPULL, GPIO12);
    gpio_clear(GPIOC, GPIO12);

    // usb pull up before usb init (so OS sees new device i think)
    gpio_set(GPIOC, GPIO11);
    gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_2_MHZ,
        GPIO_CNF_OUTPUT_PUSHPULL, GPIO11);
    usb_init(serial_read_cb);
    gpio_clear(GPIOC, GPIO11);

    // nrf init
    _nrf_init();

    // led off
    for (i = 0; i < 0x400000; i++)
        __asm__("nop");
    gpio_set(GPIOC, GPIO12);

    while(1)
    {
        usb_poll();
        _nrf_poll();
    }
}


