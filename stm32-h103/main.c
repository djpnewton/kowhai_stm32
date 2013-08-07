#include <stdlib.h>
#include <libopencm3/stm32/f1/rcc.h>
#include <libopencm3/stm32/f1/gpio.h>
#include "nrf24l01.h"
#include "usb.h"

#define PTX

void _nrf_init(void)
{
	int i;
	// http://gpio.kaltpost.de/?page_id=726
	static nrf_reg_buf addr;
	nrf_init();
	addr.data[0] = 1;
	addr.data[1] = 2;
	addr.data[2] = 3;
	addr.data[3] = 4;
	addr.data[4] = 5;
#ifdef PTX
	nrf_preset_sb(NRF_MODE_PTX, 40, 1, &addr);
#else
	nrf_preset_sb(NRF_MODE_PRX, 40, 1, &addr);
#endif
	for (i = 0; i < 0x400000; i++)
		__asm__("nop");
}

void _nrf_poll(void)
{
	int res;
	nrf_payload p;
	p.size = 1;
#ifdef PTX
	p.data[0] = 'Z';
	res = nrf_send_blocking(&p);
#else
	res = NRF_ERR_RX_FULL;
	while (res == NRF_ERR_RX_FULL)
		res = nrf_receive_blocking(&p);
#endif
	if (res == 1)
	{
		int i;
		gpio_toggle(GPIOC, GPIO12);
		for (i = 0; i < 0x400000; i++)
			__asm__("nop");
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
	//gpio_set(GPIOC, GPIO11);
	//gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_2_MHZ,
	//	GPIO_CNF_OUTPUT_PUSHPULL, GPIO11);
	//usb_init();
	gpio_clear(GPIOC, GPIO11);

	_nrf_init();	
	
	// led off
	for (i = 0; i < 0x400000; i++)
		__asm__("nop");
	gpio_set(GPIOC, GPIO12);
	
	while(1)
	{
		//usb_poll();
		_nrf_poll();
	}
}


