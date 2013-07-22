#include <stdlib.h>
#include <libopencm3/stm32/f1/rcc.h>
#include <libopencm3/stm32/f1/gpio.h>
#include "usb.h"

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
	usb_init();
	gpio_clear(GPIOC, GPIO11);

	// led off
	for (i = 0; i < 0x400000; i++)
		__asm__("nop");
	gpio_set(GPIOC, GPIO12);

	usb_poll_forever();
}


