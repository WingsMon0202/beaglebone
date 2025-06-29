#include <linux/platform_device.h>   // For platform device and driver structures
#include <linux/of.h>                // For Device Tree parsing (of_property_read_string)
#include <linux/gpio/consumer.h>     // For GPIO descriptor API: gpiod_get/set_value
#include <linux/module.h>            // For module macros: MODULE_LICENSE, EXPORT_SYMBOL, etc.
#include <linux/kernel.h>            // For logging: pr_info, pr_err

// GPIO descriptor for the LED
static struct gpio_desc *led_desc;

// Track the current state of the LED (true = ON, false = OFF)
static bool led_state = false;

/**
 * get_led_status - Return the current logic level of the LED GPIO
 * 
 * Return:
 *   1 if LED is ON (GPIO high)
 *   0 if LED is OFF (GPIO low)
 */
int get_led_status(void)
{
    return gpiod_get_value(led_desc);
}
EXPORT_SYMBOL(get_led_status);  // Export this function to be used by other modules

/**
 * gpio_led_toggle - Toggle the LED GPIO and update internal state
 */
void gpio_led_toggle(void)
{
    led_state = !led_state;                      // Flip the LED state
    gpiod_set_value(led_desc, led_state);        // Apply new value to the GPIO pin
    pr_info("gpio-led: LED toggled to %s\n", led_state ? "ON" : "OFF");
}
EXPORT_SYMBOL(gpio_led_toggle);  // Export to allow other drivers to call this

/**
 * led_probe - Called when the driver is matched with a Device Tree node
 */
static int led_probe(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;
    const char *label = "gpio-led";  // Default label if not specified in DT

    // Optionally get label from Device Tree
    of_property_read_string(dev->of_node, "label", &label);

    // Request the LED GPIO as output and set initial value to LOW (0)
    led_desc = devm_gpiod_get(dev, NULL, GPIOD_OUT_LOW);
    if (IS_ERR(led_desc)) {
        dev_err(dev, "Failed to get LED GPIO descriptor\n");
        return PTR_ERR(led_desc);
    }

    pr_info("gpio-led: initialized (label: %s)\n", label);

    // Turn the LED on initially
    gpiod_set_value(led_desc, 1);
    led_state = true;

    return 0;
}

/**
 * led_remove - Called when the device is removed or driver unloaded
 */
static int led_remove(struct platform_device *pdev)
{
    pr_info("gpio-led: removed\n");
    return 0;
}

// Match table for Device Tree binding
static const struct of_device_id led_of_match[] = {
    { .compatible = "wings,gpio-led", },
    { },
};
MODULE_DEVICE_TABLE(of, led_of_match);  // Required for Device Tree support

// Define platform driver structure
static struct platform_driver led_driver = {
    .probe = led_probe,
    .remove = led_remove,
    .driver = {
        .name = "gpio_led_driver",
        .of_match_table = led_of_match,
    },
};

// Register the platform driver
module_platform_driver(led_driver);

// Module metadata
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Wings Mon");
MODULE_DESCRIPTION("Modern GPIO LED driver using descriptor API");
