#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/sensor.h>

// Stack size settings
#define SENSOR_THREAD_STACK_SIZE 512
#define OUTPUT_THREAD_STACK_SIZE 512
#define QUEUE_MAX 10

// Define stack areas for the threads
K_THREAD_STACK_DEFINE(sensor_stack, SENSOR_THREAD_STACK_SIZE);
K_THREAD_STACK_DEFINE(output_stack, OUTPUT_THREAD_STACK_SIZE);

// Declare thread data structs
static struct k_thread sensor_thread;
static struct k_thread output_thread;

// Define shared temperature value
static struct sensor_value temperature;

// Define semaphore (initial count 0 and maximum 1)
K_SEM_DEFINE(my_semaphore, 0, 1);

// Define mutex
K_MUTEX_DEFINE(my_mutex);

// Sensor thread entry point
void sensor_thread_start(void *sensor, void *arg2, void *arg3)
{
	int ret;

	// Cast sensor handle back to device struct pointer
	const struct device *mcp = (const struct device *)sensor;

	printk("Starting sensor thread\r\n");

	while (1) {

		// Sleep early to prevent fast loop on failure
		k_msleep(1000);

		// Fetch the temperature value from the sensor into the device's data structure
        ret = sensor_sample_fetch(mcp);
        if (ret < 0) {
            printk("Sample fetch error: %d\n", ret);
            continue;
        }

        // Copy the temperature value from the device's data structure into the tmp struct
		// Use the mutex to prevent other threads from accessing the variable during the update
		k_mutex_lock(&my_mutex, K_FOREVER);
        ret = sensor_channel_get(mcp, SENSOR_CHAN_AMBIENT_TEMP, &temperature);
		k_mutex_unlock(&my_mutex);
        if (ret < 0) {
            printk("Channel get error: %d\n", ret);
            continue;
        }

        // Update the semaphore to notify the other thread that data is ready
		k_sem_give(&my_semaphore);

	}
}

// Output thread entry point
void output_thread_start(void *arg1, void *arg2, void *arg3)
{
	printk("Starting output thread\r\n");

	while (1) {

		// Wait for semaphore (no need to sleep thread!)
		k_sem_take(&my_semaphore, K_FOREVER);

		// Use (print) the data, protected by a mutex
		k_mutex_lock(&my_mutex, K_FOREVER);
		printk("Temperature: %d.%06d\n", temperature.val1, temperature.val2);
		k_mutex_unlock(&my_mutex);
	}
}

int main(void) 
{
	int ret;
	k_tid_t sensor_tid;
	k_tid_t output_tid;

	// Get Devicetree configuration for sensor
	const struct device *const mcp = DEVICE_DT_GET(DT_ALIAS(my_mcp9808));

	// Check if the MCP9808 has been initialized (init function called)
	if (!device_is_ready(mcp)) {
		printk("Device %s is not ready.\n", mcp->name);
		return 0;
	}

	// Start the sensor thread
	sensor_tid = k_thread_create(&sensor_thread,		// Thread struct
								sensor_stack,			// Stack
								K_THREAD_STACK_SIZEOF(sensor_stack),
								sensor_thread_start,	// Entry point
								(void *)mcp,			// Pass in sensor handle
								NULL,					// arg_2
								NULL,					// arg_3
								7,						// Priority
								0,						// Options
								K_NO_WAIT);				// Delay

	// Start the output thread
	output_tid = k_thread_create(&output_thread,		// Thread struct
								output_stack,			// Stack
								K_THREAD_STACK_SIZEOF(output_stack),
								output_thread_start,	// Entry point
								NULL,					// arg_1
								NULL,					// arg_2
								NULL,					// arg_3
								8,						// Priority
								0,						// Options
								K_NO_WAIT);				// Delay

	// Do nothing
	while (1) {
		k_msleep(1000);
	}

	return 0;
}