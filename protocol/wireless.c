#include "wireless.h"
#include "nrf24l01.h"
#include <libopencm3/stm32/f1/gpio.h>

#define AG_CMD_DATA 0
#define AG_CMD_ACKWAIT 1

struct ag_payload_t
{
    uint8_t cmd;
    uint8_t data;
};

static void _nrf_init(int mode)
{
    int i;
    int nrf_mode = NRF_MODE_PTX;
    // http://gpio.kaltpost.de/?page_id=726
    static nrf_reg_buf addr;
    nrf_init();
    addr.data[0] = 1;
    addr.data[1] = 2;
    addr.data[2] = 3;
    addr.data[3] = 4;
    addr.data[4] = 5;

    if (mode == AG_MODE_SLAVE)
        mode = NRF_MODE_PRX;
    nrf_preset_esbpl(mode, 40, sizeof(struct ag_payload_t), 3, NRF_RT_DELAY_250, &addr);

    // wait for radio to power up
    for (i = 0; i < 0x400000; i++)
        __asm__("nop");
}

static void _nrf_poll(void)
{
    int res;
    nrf_payload p;
    p.size = sizeof(struct ag_payload_t);
    res = nrf_receive_blocking(&p);
    if (res == sizeof(struct ag_payload_t))
    {
        struct ag_payload_t* ag_payload = p.data;
        // echo back data packets via nrf ack
        if (ag_payload->cmd == AG_CMD_DATA)
        {
            nrf_write_ack_pl(&p, 0);
            gpio_toggle(GPIOC, GPIO12);
        }
    }
}

static int _mode;

void wireless_init(int mode)
{
    _mode = mode;
    _nrf_init(mode);
}

void wireless_send_serial_char(char c)
{
    int res;
    nrf_payload p;
    struct ag_payload_t* ag_payload = (struct ag_payload_t*)p.data;
    p.size = sizeof(struct ag_payload_t);
    ag_payload->cmd = AG_CMD_DATA;
    ag_payload->data = c;
    res = nrf_send_blocking(&p);
    if (res == sizeof(struct ag_payload_t))
    {
        ag_payload->cmd = AG_CMD_ACKWAIT;
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

void wireless_poll(void)
{
    if (_mode == AG_MODE_SLAVE)
        _nrf_poll();
}

