#include <linux/module.h>           // For module macros: MODULE_LICENSE, MODULE_AUTHOR, MODULE_DESCRIPTION, EXPORT_SYMBOL
#include <linux/init.h>             // For module initialization and cleanup (init/exit), though not directly used here

#include <linux/of.h>               // For reading properties from the Device Tree: of_property_read_string()
#include <linux/of_gpio.h>          // For legacy Device Tree GPIO bindings (optional, not directly used with descriptor API)

#include <linux/gpio/consumer.h>    // For the modern GPIO descriptor API: devm_gpiod_get(), gpiod_to_irq(), gpiod_get_value()
#include <linux/interrupt.h>        // For interrupt handling: irqreturn_t, devm_request_irq(), IRQF_TRIGGER_FALLING, IRQ_HANDLED
#include <linux/platform_device.h>  // For platform driver support: platform_device, platform_driver, .probe, .remove


// GPIO descriptor for the button
static struct gpio_desc *button_desc;

// IRQ number assigned to the button GPIO
static int irq_number;

// Label of the button (can be overridden via Device Tree)
static const char *button_label = "gpio-button";

// External LED toggle function provided by the LED driver
extern void gpio_led_toggle(void);

/**
 * button_isr - Interrupt handler for the GPIO button
 * @irq: IRQ number triggered
 * @dev_id: Pointer to the platform device
 *
 * This function is executed when the button is pressed
 * (IRQ triggered on falling edge). It toggles the LED.
 *
 * Return: IRQ_HANDLED after successful handling.
 */
static irqreturn_t button_isr(int irq, void *dev_id)
{
    struct platform_device *pdev = dev_id;
    struct device *dev = &pdev->dev;

    dev_info(dev, "Interrupt triggered on IRQ %d (button: '%s')\n", irq, button_label);
    gpio_led_toggle();  // Toggle the LED state
    return IRQ_HANDLED;
}

/**
 * button_probe - Called when the device is matched and initialized
 * @pdev: Pointer to the platform device structure
 *
 * Tasks:
 * - Read button label from Device Tree (optional)
 * - Request the GPIO descriptor
 * - Convert GPIO to IRQ number
 * - Register an interrupt handler (ISR)
 *
 * Return: 0 on success, negative error code on failure
 */
static int button_probe(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;
    int ret;

    dev_info(dev, "Probing gpio-button device...\n");

    // Optionally read the button label from the device tree
    of_property_read_string(dev->of_node, "label", &button_label);

    // Get the GPIO descriptor from Device Tree, as input
    button_desc = devm_gpiod_get(dev, NULL, GPIOD_IN);
    if (IS_ERR(button_desc)) {
        dev_err(dev, "Failed to get BUTTON GPIO descriptor\n");
        return PTR_ERR(button_desc);
    }

    // Map the GPIO to an IRQ number
    irq_number = gpiod_to_irq(button_desc);
    if (irq_number < 0) {
        dev_err(dev, "Failed to map GPIO to IRQ\n");
        return irq_number;
    }

    // Register interrupt handler for falling edge (button press)
    ret = devm_request_irq(dev, irq_number, button_isr,
                           IRQF_TRIGGER_FALLING,   // Trigger on falling edge
                           "gpio_button_irq",      // Name shown in /proc/interrupts
                           pdev);                  // Device ID passed to ISR
    if (ret) {
        dev_err(dev, "Failed to request IRQ\n");
        return ret;
    }

    dev_info(dev, "Button IRQ handler registered\n");
    return 0;
}

/**
 * button_remove - Called when the device is removed
 * @pdev: Pointer to the platform device structure
 *
 * Return: 0
 */
static int button_remove(struct platform_device *pdev)
{
    pr_info("gpio-button: Device removed\n");
    return 0;
}

// Device Tree match table for compatible strings
static const struct of_device_id button_of_match[] = {
    { .compatible = "wings,gpio-button", },
    { },
};
MODULE_DEVICE_TABLE(of, button_of_match);

// Platform driver structure
static struct platform_driver button_driver = {
    .probe = button_probe,
    .remove = button_remove,
    .driver = {
        .name = "gpio_button_driver",
        .of_match_table = button_of_match,
    },
};

// Register the driver with the platform bus
module_platform_driver(button_driver);

/**
 * get_button_status - Returns current state of the button
 *
 * Return: 0 if button not pressed (high), 1 if pressed (low - active low)
 */
int get_button_status(void)
{
    return gpiod_get_value(button_desc);
}
EXPORT_SYMBOL(get_button_status);  // Make this symbol visible to other kernel modules

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Wings Mon");
MODULE_DESCRIPTION("GPIO Button Interrupt Driver");