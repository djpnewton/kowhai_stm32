/*
 * This file is part of the libopencm3 project.
 *
 * Copyright (C) 2010 Gareth McMullin <gareth@blacksphere.co.nz>
 * Copyright (C) 2013 Daniel Newton <djpnewton@gmail.com>
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <libopencm3/stm32/f4/rcc.h>
#include <libopencm3/stm32/f4/gpio.h>
#include "usb.h"

void delay(void)
{
    int i;
    for (i = 0; i < 0x400000; i++)
        __asm__("nop");
}
int main(void)
{
    rcc_clock_setup_hse_3v3(&hse_8mhz_3v3[CLOCK_3V3_120MHZ]);

    rcc_peripheral_enable_clock(&RCC_AHB1ENR, RCC_AHB1ENR_IOPAEN);
    rcc_peripheral_enable_clock(&RCC_AHB1ENR, RCC_AHB1ENR_IOPBEN);
    rcc_peripheral_enable_clock(&RCC_AHB1ENR, RCC_AHB1ENR_IOPCEN);
    rcc_peripheral_enable_clock(&RCC_AHB1ENR, RCC_AHB1ENR_IOPDEN);
    rcc_peripheral_enable_clock(&RCC_AHB2ENR, RCC_AHB2ENR_OTGFSEN);

    // single led on
    gpio_mode_setup(GPIOD, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE,
            GPIO12 | GPIO13 | GPIO14 | GPIO15);
    gpio_set(GPIOD, GPIO12);

    // init usb
    gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE,
            GPIO9 | GPIO11 | GPIO12);
    gpio_set_af(GPIOA, GPIO_AF10, GPIO9 | GPIO11 | GPIO12);
    usb_init();

    // flash leds then off
    delay();
    gpio_set(GPIOD, GPIO13);
    delay();
    gpio_set(GPIOD, GPIO14);
    delay();
    gpio_set(GPIOD, GPIO15);
    delay();
    gpio_clear(GPIOD, GPIO12 | GPIO13 | GPIO14 | GPIO15);

    usb_poll_forever();
}
