#include <linux/module.h>
#include <linux/init.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>

extern void gpio_led_toggle(void);  // Gọi hàm từ led driver

static int gpio_button;
static int irq_number;

static irqreturn_t button_isr(int irq, void *dev_id)
{
    gpio_led_toggle();
    return IRQ_HANDLED;
}

static int button_probe(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;
    const char *label;
    int ret;

    gpio_button = of_get_named_gpio(dev->of_node, "gpios", 0);
    if (!gpio_is_valid(gpio_button)) {
        dev_err(dev, "Invalid GPIO for Button\n");
        return -EINVAL;
    }

    if (of_property_read_string(dev->of_node, "label", &label))
        label = "gpio-button";

    ret = devm_gpio_request_one(dev, gpio_button, GPIOF_IN, label);
    if (ret) {
        dev_err(dev, "Failed to request button GPIO\n");
        return ret;
    }

    irq_number = gpio_to_irq(gpio_button);
    if (irq_number < 0) {
        dev_err(dev, "Failed to get IRQ number\n");
        return irq_number;
    }

    ret = devm_request_irq(dev, irq_number, button_isr,
                           IRQF_TRIGGER_FALLING | IRQF_SHARED,
                           "gpio_button_irq", (void *)pdev);
    if (ret) {
        dev_err(dev, "Failed to request IRQ\n");
        return ret;
    }

    dev_info(dev, "gpio-button: IRQ %d assigned\n", irq_number);
    return 0;
}

static int button_remove(struct platform_device *pdev)
{
    pr_info("gpio-button: removed\n");
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
MODULE_DESCRIPTION("Button interrupt driver with LED toggle interface");
