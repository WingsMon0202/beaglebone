#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/gpio/consumer.h>
#include <linux/module.h>
#include <linux/kernel.h>

static struct gpio_desc *led_desc;
static bool led_state = false;

int get_led_status(void)
{
    return gpiod_get_value(led_desc);
}
EXPORT_SYMBOL(get_led_status);

void gpio_led_toggle(void)
{
    led_state = !led_state;
    gpiod_set_value(led_desc, led_state);
    pr_info("gpio-led: LED toggled to %s\n", led_state ? "ON" : "OFF");
}
EXPORT_SYMBOL(gpio_led_toggle);

static int led_probe(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;
    const char *label = "gpio-led";

    of_property_read_string(dev->of_node, "label", &label);

    led_desc = devm_gpiod_get(dev, NULL, GPIOD_OUT_LOW);
    if (IS_ERR(led_desc)) {
        dev_err(dev, "Failed to get LED GPIO descriptor\n");
        return PTR_ERR(led_desc);
    }

    pr_info("gpio-led: initialized\n");
    gpiod_set_value(led_desc, 1);  // Turn LED on initially
    return 0;
}

static int led_remove(struct platform_device *pdev)
{
    pr_info("gpio-led: removed\n");
    return 0;
}

static const struct of_device_id led_of_match[] = {
    { .compatible = "wings,gpio-led", },
    { },
};
MODULE_DEVICE_TABLE(of, led_of_match);

static struct platform_driver led_driver = {
    .probe = led_probe,
    .remove = led_remove,
    .driver = {
        .name = "gpio_led_driver",
        .of_match_table = led_of_match,
    },
};
module_platform_driver(led_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Wings Mon");
MODULE_DESCRIPTION("Modern GPIO LED driver using descriptor API");
