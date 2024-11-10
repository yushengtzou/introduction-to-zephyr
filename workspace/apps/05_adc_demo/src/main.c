#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/adc.h>

// Settings
static const int32_t sleep_time_ms = 1000;
// static const struct adc_dt_spec adc = ADC_DT_SPEC_GET(DT_ALIAS(my_adc));
static const struct adc_dt_spec adc = ADC_DT_SPEC_GET(DT_PATH(zephyr_user));

int main(void)
{
	int ret;
	uint16_t buf;

	// Buffer and options for ADC (defined in adc.h)
	struct adc_sequence seq = {
		.buffer = &buf,
		.buffer_size = sizeof(buf),
	};

	// Make sure that the ADC was initialized
	if (!adc_is_ready_dt(&adc)) {
		printk("ADC peripheral is not ready\r\n");
		return 0;
	}

	// Configure ADC channel
	ret = adc_channel_setup_dt(&adc);
	if (ret < 0) {
		printk("Could not set up ADC\r\n");
		return 0;
	}

	// Copy channels, resolution, and oversampling from DT spec to sequence
	adc_sequence_init_dt(&adc, &seq);

	// Do forever
	while (1) {

		// Sample ADC
		ret = adc_read_dt(&adc, &seq);
		if (ret < 0) {
			printk("Could not read ADC: %d\r\n", ret);
			continue;
		}

		// Print ADC value
		printk("%u\r\n", buf);

		// TEST
		printk("%d, %d\r\n", adc.vref_mv, adc.resolution);

		// Sleep
		k_msleep(sleep_time_ms);
	}

	return 0;
}
