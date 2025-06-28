#include <linux/module.h>
#include <linux/init.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/gpio/consumer.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>

static struct gpio_desc *button_desc;
static int irq_number;
static const char *button_label = "gpio-button";

extern void gpio_led_toggle(void);

static irqreturn_t button_isr(int irq, void *dev_id)
{
    struct platform_device *pdev = dev_id;
    struct device *dev = &pdev->dev;

    dev_info(dev, "Interrupt triggered on IRQ %d (button: '%s')\n", irq, button_label);
    gpio_led_toggle();
    return IRQ_HANDLED;
}

static int button_probe(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;
    int ret;

    dev_info(dev, "Probing gpio-button device...\n");

    // Optional: Read label from Device Tree
    of_property_read_string(dev->of_node, "label", &button_label);

    // Request GPIO descriptor
    button_desc = devm_gpiod_get(dev, NULL, GPIOD_IN);
    if (IS_ERR(button_desc)) {
        dev_err(dev, "Failed to get BUTTON GPIO descriptor\n");
        return PTR_ERR(button_desc);
    }

    // Map GPIO to IRQ
    irq_number = gpiod_to_irq(button_desc);
    if (irq_number < 0) {
        dev_err(dev, "Failed to map GPIO to IRQ\n");
        return irq_number;
    }

    // Register IRQ handler
    ret = devm_request_irq(dev, irq_number, button_isr,
                           IRQF_TRIGGER_FALLING,
                           "gpio_button_irq", pdev);
    if (ret) {
        dev_err(dev, "Failed to request IRQ\n");
        return ret;
    }

    dev_info(dev, "Button IRQ handler registered\n");
    return 0;
}

static int button_remove(struct platform_device *pdev)
{
    pr_info("gpio-button: Device removed\n");
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

int get_button_status(void)
{
    return gpiod_get_value(button_desc);
}
EXPORT_SYMBOL(get_button_status);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Wings Mon");
MODULE_DESCRIPTION("Modern GPIO Button Interrupt Driver using descriptor API");
