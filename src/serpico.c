/**
 * SERPICO alias of Studying Electronics and Robotics with PICO, is a project of
 * Robot based on Raspberry PI Pico board.
 * This code use two PWM Slice, both channels for each slice, one of which with 
 * inverted logic, are used to control a DRV8833 that drives the two DC motors.
 * 
 * Name          : serpico.c
 * @author       : Roberto D'Amico (bobboteck)
 * Last modified : 2021/03/22
 * Revision      : 1.3.0
 * 
 * Modification History:
 * Date         Version Modified By     Description
 * 2021-03-22   1.3.0   Roberto D'Amico First test with bluetooth LE on UART0
 * 2021-03-21   1.2.1   Roberto D'Amico Simply move the GPIO to free the UART GPIOs
 * 2021-03-01   1.2.0   Roberto D'Amico Implemented sensor limit and move
 * 2021-02-19   1.1.0   Roberto D'Amico Test US HC-SR04 Sensor - OK!
 * 2021-02-16   1.0.0   Roberto D'Amico Move forward and backward the Robot
 * 
 * The MIT License (MIT)
 *
 * This file is part of the SERPICO Project (https://github.com/bobboteck/SERPICO).
 * Copyright (c) 2021 Roberto D'Amico (bobboteck).
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
#include "hardware/uart.h"
#include "hardware/irq.h"

#define UART_ID uart0
#define BAUD_RATE 9600
#define UART_TX_PIN 0
#define UART_RX_PIN 1

const uint LED_PIN = 25;
const uint US_TRIGGER = 6;
const uint US_ECHO = 7;

uint32_t usMeter(void);
void turn(uint s0, uint s1);
void onUart0Receive(void);

int main()
{
    stdio_init_all();

    // Set up our UART with the required speed.
    uart_init(UART_ID, BAUD_RATE);

    // Set the TX and RX pins by using the function select on the GPIO
    // Set datasheet for more information on function select
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    
    // And set up and enable the interrupt handlers
    //irq_set_exclusive_handler(UART_IRQ, onUart0Receive);
    irq_set_exclusive_handler(UART0_IRQ, onUart0Receive);
    irq_set_enabled(UART0_IRQ, true);

    // Now enable the UART to send interrupts - RX only
    uart_set_irq_enables(UART_ID, true, false);


    // Led on PICO
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    // Pin used to drive the Trigger of HC-SR04
    gpio_init(US_TRIGGER);
    gpio_set_dir(US_TRIGGER, GPIO_OUT);
    gpio_put(US_TRIGGER, 0);
    // Pin used to read the Echo of HC-SR04
    gpio_init(US_ECHO);
    gpio_set_dir(US_ECHO, GPIO_IN);

    // GPIO 2, 3, 4 and 5 are allocated as PWM
    gpio_set_function(2, GPIO_FUNC_PWM);
    gpio_set_function(3, GPIO_FUNC_PWM);
    gpio_set_function(4, GPIO_FUNC_PWM);
    gpio_set_function(5, GPIO_FUNC_PWM);

    // Find out which PWM slice is connected to GPIO 2 (Slice 1) and GPIO4 (Slice 2)
    uint slice1 = pwm_gpio_to_slice_num(2);
    uint slice2 = pwm_gpio_to_slice_num(4);

    // Set Free running, No phase correct, int 1, frac 4 and channel B inverted for Slice 1
    pwm_set_clkdiv_mode(slice1, PWM_DIV_FREE_RUNNING);
    pwm_set_phase_correct(slice1, false);
    pwm_set_clkdiv_int_frac(slice1, 1, 4);
    pwm_set_output_polarity(slice1, false, true);
    // Set Free running, No phase correct, int 1, frac 4 and channel B inverted for Slice 2
    pwm_set_clkdiv_mode(slice2, PWM_DIV_FREE_RUNNING);
    pwm_set_phase_correct(slice2, false);
    pwm_set_clkdiv_int_frac(slice2, 1, 4);
    pwm_set_output_polarity(slice2, false, true);

    // Set the TOP register to 5000, which with the system frequency at 125MHz, corresponds to a frequency of 20KHz for PWM of Slice 1
    pwm_set_wrap(slice1, 5000);
    pwm_set_both_levels(slice1, 2500, 2500);
    // Set the TOP register to 5000, which with the system frequency at 125MHz, corresponds to a frequency of 20KHz for PWM of Slice 2
    pwm_set_wrap(slice2, 5000);
    pwm_set_both_levels(slice2, 2500, 2500);

    // Enable PWM running
    pwm_set_enabled(slice1, true);
    pwm_set_enabled(slice2, true);

    // Print over serial the system clock frequency
    uint32_t clockHz = clock_get_hz(clk_sys);
    printf("clk_sys: %dHz\n", clockHz);

    // Flash on board led five times
    for(int i=0;i<5;i++)
    {
        gpio_put(LED_PIN, 1);
        sleep_ms(250);
        gpio_put(LED_PIN, 0);
        sleep_ms(250);
    }

    uint16_t pwmDuty = 2500;
    uint32_t obstacleDistance = 0;

    while (1)
    {
        // Read the distance with possible obstacle
        obstacleDistance = usMeter();
        // Check the measure
        if(obstacleDistance > 10)
        {
            // Calculates the value to be assigned to the Duty Cycle control register
            pwmDuty = (obstacleDistance * 10) + 3000;
            // Check not to exceed the limit of 5000
            if(pwmDuty > 5000) pwmDuty=5000;
            // Set the Duty Cycle on all Slice
            //pwm_set_both_levels(slice1, pwmDuty, pwmDuty);
            //pwm_set_both_levels(slice2, pwmDuty, pwmDuty);
        }
        else
        {
            // Call function to turn the Robot
            //turn(slice1, slice2);
        }
        
        // Print debug information over serial
        //printf("Obstacle distance: %d cm - PWM Duty: %d\n", obstacleDistance, pwmDuty);
        printf("US:%d\n", obstacleDistance);
        printf("DC:%d\n", pwmDuty);
        sleep_ms(500);
    }
}

/**
 * Read the distance with possible obstacle
 * 
 */
uint32_t usMeter(void)
{
    uint32_t start_echo_time=0, end_echo_time=0, distance=0;

    // Send trigger pulse
    sleep_us(2);
    gpio_put(US_TRIGGER, 1);
    sleep_us(5);
    gpio_put(US_TRIGGER, 0);
    
    // Wait for the Echo output pin to go high and store the time
    while(gpio_get(US_ECHO) == 0)
    {
        start_echo_time = time_us_32();
    }

    end_echo_time = start_echo_time;

    // Wait for the Echo output pin to go low and store the time
    while(gpio_get(US_ECHO) == 1 && (end_echo_time - start_echo_time) < 11600)
    {
        end_echo_time = time_us_32();
    }

    // Calculate the distance
    distance = ((end_echo_time - start_echo_time) * 0.0343) / 2;

    //printf("Start: %d - End: %d - Distance: %d cm\n",start_echo_time, end_echo_time, distance);

    return distance;
}

/**
 * Turn Robot 
 * 
 * @param s0 The Slice for the Right Motor
 * @param S1 The Slice for the Left Motor
 */
void turn(uint s0, uint s1)
{
    // Stop the motor
    pwm_set_both_levels(s0, 2500, 2500);
    pwm_set_both_levels(s1, 2500, 2500);

    // Turn
    pwm_set_both_levels(s0, 1800, 1800);
    pwm_set_both_levels(s1, 3000, 3000);

    // Wait while the Robot turns 
    sleep_ms(1000);

    // Stop the motor
    pwm_set_both_levels(s0, 2500, 2500);
    pwm_set_both_levels(s1, 2500, 2500);
}


void onUart0Receive(void)
{
    //char str[10];

    while (uart_is_readable(UART_ID))
    {
        uint8_t ch = uart_getc(UART_ID);

        //strcat(str,ch);

        if(ch=='L')
        {
            gpio_put(LED_PIN, 0);
        }
        else if(ch=='E' || ch=='D' || ch==':' || ch=='0')
        {
            gpio_put(LED_PIN, 1);
        }

        uart_putc(UART_ID, ch);
    }
}
