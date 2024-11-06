#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/adc.h>

// Settings
static const uint8_t adc_channel = 4;
static const uint8_t adc_resolution = 12;

// ADC settings
static const struct adc_channel_cfg adc_cfg = {
    .gain = ADC_GAIN_1,
    .reference = ADC_REF_INTERNAL,
    .acquisition_time = ADC_ACQ_TIME_DEFAULT,
    .channel_id = adc_channel,
    .differential = 0,
};

int main(void)
{
	int ret;
	int state = 0;

	// Make sure that the GPIO was initialized
	if (!gpio_is_ready_dt(&led)) {
		return 0;
	}

	// Set the GPIO as output
	ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT);
	if (ret < 0) {
		return 0;
	}

	// Do forever
	while (1) {

		// Change the state of the pin and print
		state = !state;
		printk("LED state: %d\r\n", state);
		
		// Set pin state
		ret = gpio_pin_set_dt(&led, state);
		if (ret < 0) {
			return 0;
		}

		// Sleep
		k_msleep(sleep_time_ms);
	}

	return 0;
}
