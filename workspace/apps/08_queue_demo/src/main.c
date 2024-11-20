#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/console/console.h>

// Stack size settings
#define BLINK_THREAD_STACK_SIZE 512
#define INPUT_THREAD_STACK_SIZE 512
#define QUEUE_MAX 10

// Blink settings
static const int32_t blink_max_ms = 2000;
static const int32_t blink_min_ms = 0;

// Define stack areas for the threads
K_THREAD_STACK_DEFINE(blink_stack, BLINK_THREAD_STACK_SIZE);
K_THREAD_STACK_DEFINE(input_stack, INPUT_THREAD_STACK_SIZE);

// Declare thread data structs
static struct k_thread blink_thread;
static struct k_thread input_thread;

// Define queue
K_MSGQ_DEFINE(my_queue, sizeof(int8_t), QUEUE_MAX, 1);

// Get LED struct from Devicetree
const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_ALIAS(my_led), gpios);

// Console input thread entry point
void input_thread_start(void *arg1, void *arg2, void *arg3)
{
	int ret;
	int8_t val;

	printk("Starting input thread\r\n");

	while (1) {

		// Get line from console (blocking)
		const char *line = console_getline();

		// See if first character is + or -
		if (line[0] == '+') {
			val = 1;
		} else if (line[0] == '-') {
			val = -1;
		} else {
			continue;
		}

		// Put message on queue
		printk("Putting val %d\r\n", val);
		ret = k_msgq_put(&my_queue, &val, K_NO_WAIT);
		if (ret < 0) {
			printk("Queue full\r\n");
		}
	}
}

// Blink thread entry point
void blink_thread_start(void *arg1, void *arg2, void *arg3)
{
	int ret;
	int state = 0;
	int32_t blink_sleep_ms = 500;
	int8_t val = 0;

	printk("Starting blink thread\r\n");

	while (1) {

		// Get message from queue
		ret = k_msgq_get(&my_queue, &val, K_NO_WAIT);

		// Increase or decrease wait time as requested and bound sleep time
		if (ret == 0) {
			printk("Received val: %d\r\n", val);
			blink_sleep_ms += (int32_t)val * 100;
			if (blink_sleep_ms > blink_max_ms) {
				blink_sleep_ms = blink_max_ms;
			} else if (blink_sleep_ms < blink_min_ms) {
				blink_sleep_ms = blink_min_ms;
			}
			printk("Updating blink sleep to: %d\r\n", blink_sleep_ms);
		}

		// Change the state of the pin and print
		state = !state;
		
		// Set pin state
		ret = gpio_pin_set_dt(&led, state);
		if (ret < 0) {
			printk("Error: could not toggle pin\r\n");
		}

		// Sleep for specified time
		k_msleep(blink_sleep_ms);
	}
}

int main(void) 
{
	int ret;
	k_tid_t input_tid;
	k_tid_t blink_tid;

	// Make sure that the GPIO was initialized
	if (!gpio_is_ready_dt(&led)) {
		return 0;
	}

	// Set the GPIO as output
	ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT);
	if (ret < 0) {
		return 0;
	}

	// Initialize the console
	console_getline_init();

	// Start the input thread
	input_tid = k_thread_create(&blink_thread,		// Thread struct
								blink_stack,		// Stack
								K_THREAD_STACK_SIZEOF(blink_stack),
								input_thread_start,	// Entry point
								NULL,				// arg_1
								NULL,				// arg_2
								NULL,				// arg_3
								7,					// Priority
								0,					// Options
								K_NO_WAIT);			// Delay

	// Start the blink thread
	blink_tid = k_thread_create(&input_thread,		// Thread struct
								input_stack,		// Stack
								K_THREAD_STACK_SIZEOF(input_stack),
								blink_thread_start,	// Entry point
								NULL,				// arg_1
								NULL,				// arg_2
								NULL,				// arg_3
								8,					// Priority
								0,					// Options
								K_NO_WAIT);			// Delay

	// Do nothing
	while (1) {
		k_msleep(1000);
	}

	return 0;
}