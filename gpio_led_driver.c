#include <linux/module.h>
#include <linux/init.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/gpio.h>
#include <linux/platform_device.h>

int gpio_led;
                 
EXPORT_SYMBOL(gpio_led);      


static const char *led_label;
static int led_probe(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;
    int ret;

    dev_info(dev, "led driver: probe started\n");

    gpio_led = of_get_named_gpio(dev->of_node, "gpios", 0);
    if (!gpio_is_valid(gpio_led)) {
        dev_err(dev, "led driver: invalid GPIO\n");
        return -EINVAL;
    }
    dev_info(dev, "led driver: acquired GPIO %d\n", gpio_led);

    if (of_property_read_string(dev->of_node, "label", &led_label) == 0)
        dev_info(dev, "led driver: label = %s\n", led_label);
    else {
        led_label = "gpio-led";
        dev_info(dev, "led driver: no label found, using default\n");
    }

    ret = devm_gpio_request_one(dev, gpio_led, GPIOF_OUT_INIT_HIGH, led_label);
    if (ret) {
        dev_err(dev, "led driver: failed to request GPIO %d\n", gpio_led);
        return ret;
    }

    dev_info(dev, "led driver: LED turned ON at GPIO %d\n", gpio_led);
    return 0;
}

static int led_remove(struct platform_device *pdev)
{
    dev_info(&pdev->dev, "led driver: module removed, turning OFF LED\n");
    gpio_set_value(gpio_led, 0);
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
MODULE_DESCRIPTION("Simple GPIO LED driver with English debug logs");

