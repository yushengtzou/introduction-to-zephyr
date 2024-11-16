#include <stdio.h>
#include <zephyr/kernel.h>

#include "button.h"

// Settings
static const int32_t sleep_time_ms = 100;
static const struct device *btn = DEVICE_DT_GET(DT_ALIAS(my_button));

int main(void)
{
    int ret;
    uint8_t state;

    // Make sure that the button was initialized
    if (!device_is_ready(btn)) {
        printk("ERROR: button not ready\r\n");
        return 0;
    }

    // Get the API for the button
    const struct button_api *btn_api = (const struct button_api *)btn->api;

    // Do forever
    while (1) {
        
        // Poll button state
        ret = btn_api->get(btn, &state);
        if (ret < 0) {
            printk("Error %d: failed to read button pin\r\n", ret);
            continue;
        }
        printk("Button state: %d\r\n", state);

        // Sleep
        k_msleep(sleep_time_ms);
    }

    return 0;
}