#include <linux/module.h>
#include <linux/init.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>

extern void gpio_led_toggle(void);  // Declared from external LED driver

static int gpio_button;
static int irq_number;
static const char *button_label = "gpio-button";  // Label used in logs

// ISR: Called when button is pressed (falling edge)
static irqreturn_t button_isr(int irq, void *dev_id)
{
    struct platform_device *pdev = dev_id;
    struct device *dev = &pdev->dev;

    dev_info(dev, "Interrupt triggered on IRQ %d (button: '%s')\n", irq, button_label);

    gpio_led_toggle();  // Call external LED toggle function
    return IRQ_HANDLED;
}

static int button_probe(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;
    int ret;

    dev_info(dev, "Probing gpio-button device...\n");

    // Get GPIO from Device Tree
    gpio_button = of_get_named_gpio(dev->of_node, "gpios", 0);
    if (!gpio_is_valid(gpio_button)) {
        dev_err(dev, "Invalid GPIO provided in Device Tree\n");
        return -EINVAL;
    }
    dev_info(dev, "Button GPIO = %d\n", gpio_button);

    // Optional: Read label from Device Tree
    if (of_property_read_string(dev->of_node, "label", &button_label))
        button_label = "gpio-button";
    dev_info(dev, "Button label = '%s'\n", button_label);

    // Request GPIO manually instead of deprecated devm_gpio_request_one()
    ret = devm_gpio_request(dev, gpio_button, button_label);
    if (ret) {
        dev_err(dev, "Failed to request GPIO %d\n", gpio_button);
        return ret;
    }

    // Configure GPIO direction as input
    ret = gpio_direction_input(gpio_button);
    if (ret) {
        dev_err(dev, "Failed to set GPIO %d as input\n", gpio_button);
        return ret;
    }

    // Get IRQ number associated with the GPIO
    irq_number = gpio_to_irq(gpio_button);
    if (irq_number < 0) {
        dev_err(dev, "Failed to map GPIO %d to IRQ\n", gpio_button);
        return irq_number;
    }
    dev_info(dev, "Mapped GPIO %d to IRQ %d\n", gpio_button, irq_number);

    // Register IRQ handler
    ret = devm_request_irq(dev, irq_number, button_isr,
                           IRQF_TRIGGER_FALLING,  // Trigger on falling edge (button press)
                           "gpio_button_irq", (void *)pdev);
    if (ret) {
        dev_err(dev, "Failed to request IRQ %d\n", irq_number);
        return ret;
    }

    dev_info(dev, "IRQ handler registered successfully for button\n");
    return 0;
}

static int button_remove(struct platform_device *pdev)
{
    pr_info("gpio-button: Device removed\n");
    return 0;
}

// Match compatible string with Device Tree
static const struct of_device_id button_of_match[] = {
    { .compatible = "wings,gpio-button", },
    { },
};
MODULE_DEVICE_TABLE(of, button_of_match);

// Register platform driver
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
    return gpio_get_value(gpio_button);
}
EXPORT_SYMBOL(get_button_status);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Wings Mon");
MODULE_DESCRIPTION("GPIO Button Interrupt Driver using GPIO + IRQ + external LED toggle (no deprecated APIs)");
