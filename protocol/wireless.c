#include "wireless.h"
#include "nrf24l01.h"
#include <libopencm3/stm32/f1/gpio.h>
#include <string.h>

#define AG_CMD_DATA 0
#define AG_CMD_ACKWAIT 1
#define AG_CMD_ADDR 2

struct ag_payload_t
{
    uint8_t cmd;
    uint8_t data;
};

static int _mode;
static uint8_t _discovery_address[] = {9, 9, 0, 5, 5};
static uint8_t _addr[AG_ADDR_LEN];

// this nrf library howto at http://gpio.kaltpost.de/?page_id=726
static void _nrf_init(int mode, uint8_t addr[AG_ADDR_LEN])
{
    int i;
    int nrf_mode = NRF_MODE_PTX;
    static nrf_reg_buf buf;

    // init nrf module
    nrf_init();

    // setup enhanced shockburst with ack payload
    if (mode == AG_MODE_SLAVE)
        nrf_mode = NRF_MODE_PRX;
    memcpy(_addr, addr, AG_ADDR_LEN);
    memcpy(buf.data, addr, AG_ADDR_LEN);
    nrf_preset_esbpl(nrf_mode, 40, sizeof(struct ag_payload_t), 3, NRF_RT_DELAY_250, &buf);
    // setup discovery channel for all slaves
    if (mode == AG_MODE_SLAVE)
    {
        // enable pipe1
        nrf_read_reg(NRF_REG_EN_RXADDR, &buf);
        nrf_set_reg_field(NRF_REG_EN_RXADDR, NRF_REGF_ERX_P1,  &buf, 1);
        nrf_write_reg(NRF_REG_EN_RXADDR, &buf);
        // RX_ADDR_P1 - set receive address data pipe1
        memcpy(buf.data, _discovery_address, AG_ADDR_LEN);
        nrf_write_reg(NRF_REG_RX_ADDR_P1, &buf);
    }

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
        switch (ag_payload->cmd)
        {
            case AG_CMD_DATA:
                // echo back data packets via nrf ack
                nrf_write_ack_pl(&p, 0);
                gpio_toggle(GPIOC, GPIO12);
                break;
            case AG_CMD_ADDR:
                memcpy(p.data, _addr, AG_ADDR_LEN);
                nrf_write_ack_pl(&p, 1);
                gpio_toggle(GPIOC, GPIO12);
                break;
        }
    }
}

void wireless_init(int mode, uint8_t addr[AG_ADDR_LEN])
{
    _mode = mode;
    _nrf_init(mode, addr);
}

void wireless_set_address(uint8_t addr[AG_ADDR_LEN])
{
    _nrf_init(_mode, addr);
}

void wireless_master_send_serial_char(char c)
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

void wireless_master_find_slaves(uint8_t* count, uint8_t addrs[AG_MAX_ADDRS][AG_ADDR_LEN])
{

}

void wireless_poll(void)
{
    if (_mode == AG_MODE_SLAVE)
        _nrf_poll();
}

