#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/gpio.h>
#include <linux/kernel.h>

static int gpio_led;
static bool led_state = false;

int get_led_status(void)
{
    return gpio_get_value(gpio_led);
}
EXPORT_SYMBOL(get_led_status);


void gpio_led_toggle(void)
{
    led_state = !led_state;
    gpio_set_value(gpio_led, led_state);
    pr_info("gpio-led: LED toggled to %s\n", led_state ? "ON" : "OFF");
}
EXPORT_SYMBOL(gpio_led_toggle);

static int led_probe(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;
    const char *label;
    int ret;

    gpio_led = of_get_named_gpio(dev->of_node, "gpios", 0);
    if (!gpio_is_valid(gpio_led)) {
        dev_err(dev, "Invalid GPIO for LED\n");
        return -EINVAL;
    }

    if (of_property_read_string(dev->of_node, "label", &label))
        label = "gpio-led";

    ret = devm_gpio_request_one(dev, gpio_led, GPIOF_OUT_INIT_LOW, label);
    if (ret) {
        dev_err(dev, "Failed to request LED GPIO\n");
        return ret;
    }

    pr_info("gpio-led: initialized on GPIO %d\n", gpio_led);

    gpio_set_value(gpio_led,1);

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
MODULE_DESCRIPTION("LED driver using GPIO from device tree");
