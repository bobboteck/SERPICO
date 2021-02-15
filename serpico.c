/**
 * Test PWM frequency set at 20KHz on Slice 0 on Channel A and inverted on Channel B
 * 
 * 
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
	
    // GPIO 0 and 1 they are allocated to the PWM
    gpio_set_function(0, GPIO_FUNC_PWM);
    gpio_set_function(1, GPIO_FUNC_PWM);

    gpio_set_function(2, GPIO_FUNC_PWM);
    gpio_set_function(3, GPIO_FUNC_PWM);

    // Find out which PWM slice is connected to GPIO 0 (it's slice 0)
    uint slice0 = pwm_gpio_to_slice_num(0);
    uint slice1 = pwm_gpio_to_slice_num(2);

	// Set Free running, No phase correct, int 1, frac 4 and channel B inverted
	pwm_set_clkdiv_mode(slice0, PWM_DIV_FREE_RUNNING);
	pwm_set_phase_correct(slice0, false);
	pwm_set_clkdiv_int_frac(slice0, 1, 4);
	pwm_set_output_polarity(slice0, false, true);

	pwm_set_clkdiv_mode(slice1, PWM_DIV_FREE_RUNNING);
	pwm_set_phase_correct(slice1, false);
	pwm_set_clkdiv_int_frac(slice1, 1, 4);
	pwm_set_output_polarity(slice1, false, true);


    // Set period at 5000 on 125MHz SYS Frequencies
    pwm_set_wrap(slice0, 5000);
    pwm_set_both_levels(slice0, 2500, 2500);
    
    pwm_set_wrap(slice1, 5000);
    pwm_set_both_levels(slice1, 2500, 2500);

    // Set the PWM running
    pwm_set_enabled(slice0, true);
	pwm_set_enabled(slice1, true);

	uint32_t clockHz = clock_get_hz(clk_sys);
	printf("clk_sys: %dHz\n", clockHz);

    while (1)
    {
        gpio_put(LED_PIN, 1);

		for(uint16_t pwmDuty=0; pwmDuty<5000; pwmDuty++)
		{
			pwm_set_both_levels(slice0, pwmDuty, pwmDuty);
			pwm_set_both_levels(slice1, pwmDuty, pwmDuty);
			sleep_ms(1);
			printf("pwmDuty: %d\n", pwmDuty);
		}
        
        gpio_put(LED_PIN, 0);
        
		for(uint16_t pwmDuty=5000; pwmDuty>0; pwmDuty--)
		{
			pwm_set_both_levels(slice0, pwmDuty, pwmDuty);
			pwm_set_both_levels(slice1, pwmDuty, pwmDuty);
			sleep_ms(1);
			printf("pwmDuty: %d\n", pwmDuty);
		}
    }
}
