#include <linux/module.h>
#include <linux/init.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>

static int gpio_button;
static int gpio_led = 44; // GPIO của LED từ driver trước
static int irq_number;
static bool led_on = true;

static irqreturn_t button_isr(int irq, void *dev_id)
{
    led_on = !led_on;
    gpio_set_value(gpio_led, led_on);  // đảo trạng thái LED
    pr_info("button driver: button pressed, LED is now %s\n", led_on ? "ON" : "OFF");
    return IRQ_HANDLED;
}

static int button_probe(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;
    const char *btn_label;
    int ret;

    dev_info(dev, "button driver: probe started\n");

    gpio_button = of_get_named_gpio(dev->of_node, "gpios", 0);
    if (!gpio_is_valid(gpio_button)) {
        dev_err(dev, "button driver: invalid GPIO\n");
        return -EINVAL;
    }
    dev_info(dev, "button driver: acquired GPIO %d\n", gpio_button);

    if (of_property_read_string(dev->of_node, "label", &btn_label) == 0)
        dev_info(dev, "button driver: label = %s\n", btn_label);
    else
        btn_label = "gpio-button";

    ret = devm_gpio_request_one(dev, gpio_button, GPIOF_IN, btn_label);
    if (ret) {
        dev_err(dev, "button driver: failed to request GPIO %d\n", gpio_button);
        return ret;
    }

    irq_number = gpio_to_irq(gpio_button);
    if (irq_number < 0) {
        dev_err(dev, "button driver: failed to get IRQ number\n");
        return irq_number;
    }

    dev_info(dev, "button driver: requesting IRQ %d\n", irq_number);

    ret = devm_request_irq(dev, irq_number, button_isr,
                           IRQF_TRIGGER_FALLING | IRQF_SHARED,
                           "gpio_button_irq", (void *)pdev);
    if (ret) {
        dev_err(dev, "button driver: failed to request IRQ\n");
        return ret;
    }

    dev_info(dev, "button driver: IRQ successfully requested\n");
    return 0;
}

static int button_remove(struct platform_device *pdev)
{
    dev_info(&pdev->dev, "button driver: removed\n");
    return 0;
}

static const struct of_device_id button_of_match[] = {
    { .compatible = "wings,gpio-button", },
    { },
};
MODULE_DEVICE_TABLE(of, button_of_match);

static struct platform_driver button_driver = {
    .probe = button_probe,
    .remove = button_remove,
    .driver = {
        .name = "gpio_button_driver",
        .of_match_table = button_of_match,
    },
};
module_platform_driver(button_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Wings Mon");
MODULE_DESCRIPTION("Button interrupt driver to toggle LED");

