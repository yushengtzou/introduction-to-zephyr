#include <zephyr/random/random.h>

#ifdef CONFIG_SAY_HELLO
#include "say_hello.h"
#endif

// Settings
static const int32_t sleep_time_ms = 1000;

int main(void)
{
    uint32_t rnd;
    double rnd_float;

    // Do forever
    while (1) {

        // Print random number
        rnd = sys_rand32_get();
        rnd_float = (double)rnd / (UINT32_MAX + 1.0);
        printk("Random value: %.3f\r\n", rnd_float);

#ifdef CONFIG_SAY_HELLO
        say_hello();
#endif

        // Sleep
        k_msleep(sleep_time_ms);
    }

    return 0;
}