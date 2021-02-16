/**
 * SERPICO alias of Studying Electronics and Robotics with PICO, is a project of
 * Robot based on Raspberry PI Pico board.
 * This code use two PWM Slice, both channels for each slice, one of which with 
 * inverted logic, are used to control a DRV8833 that drives the two DC motors.
 * 
 * Name          : serpico.c
 * @author       : Roberto D'Amico (bobboteck)
 * Last modified : 2021/02/16
 * Revision      : 1.0.0
 * 
 * Modification History:
 * Date         Version     Modified By		Description
 * 2021-02-16	1.0.0		Roberto D'Amico	Move forward and backward the Robot
 * 
 * The MIT License (MIT)
 *
 *  This file is part of the SERPICO Project (https://github.com/bobboteck/SERPICO).
 *	Copyright (c) 2015 Roberto D'Amico (bobboteck).
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "pico/binary_info.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"

const uint LED_PIN = 25;

int main()
{
    stdio_init_all();

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    // GPIO 0, 1, 2 and 3 are allocated as PWM
    gpio_set_function(0, GPIO_FUNC_PWM);
    gpio_set_function(1, GPIO_FUNC_PWM);
    gpio_set_function(2, GPIO_FUNC_PWM);
    gpio_set_function(3, GPIO_FUNC_PWM);

    // Find out which PWM slice is connected to GPIO 0 (Slice 0) and GPIO2 (Slice 1)
    uint slice0 = pwm_gpio_to_slice_num(0);
    uint slice1 = pwm_gpio_to_slice_num(2);

    // Set Free running, No phase correct, int 1, frac 4 and channel B inverted for Slice 0
    pwm_set_clkdiv_mode(slice0, PWM_DIV_FREE_RUNNING);
    pwm_set_phase_correct(slice0, false);
    pwm_set_clkdiv_int_frac(slice0, 1, 4);
    pwm_set_output_polarity(slice0, false, true);
    // Set Free running, No phase correct, int 1, frac 4 and channel B inverted for Slice 0
    pwm_set_clkdiv_mode(slice1, PWM_DIV_FREE_RUNNING);
    pwm_set_phase_correct(slice1, false);
    pwm_set_clkdiv_int_frac(slice1, 1, 4);
    pwm_set_output_polarity(slice1, false, true);

    // Set the TOP register to 5000, which with the system frequency at 125MHz, corresponds to a frequency of 20KHz for PWM
    pwm_set_wrap(slice0, 5000);
    pwm_set_both_levels(slice0, 2500, 2500);

    pwm_set_wrap(slice1, 5000);
    pwm_set_both_levels(slice1, 2500, 2500);

    // Enable PWM running
    pwm_set_enabled(slice0, true);
    pwm_set_enabled(slice1, true);

    uint32_t clockHz = clock_get_hz(clk_sys);
    printf("clk_sys: %dHz\n", clockHz);

    while (1)
    {
        gpio_put(LED_PIN, 1);

        // From max speed forward to max speed backward on both motors
        for(uint16_t pwmDuty=0; pwmDuty<5000; pwmDuty++)
        {
            pwm_set_both_levels(slice0, pwmDuty, pwmDuty);
            pwm_set_both_levels(slice1, pwmDuty, pwmDuty);
            sleep_ms(1);
            printf("pwmDuty: %d\n", pwmDuty);
        }

        gpio_put(LED_PIN, 0);

        // From max speed backward to max speed forward on both motors
        for(uint16_t pwmDuty=5000; pwmDuty>0; pwmDuty--)
        {
            pwm_set_both_levels(slice0, pwmDuty, pwmDuty);
            pwm_set_both_levels(slice1, pwmDuty, pwmDuty);
            sleep_ms(1);
            printf("pwmDuty: %d\n", pwmDuty);
        }
    }
}