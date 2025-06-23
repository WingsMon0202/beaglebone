#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>

static int led_gpio = -1;
static int btn_gpio = -1;
static int irq_number = -1;

// IRQ handler for button press/release
static irqreturn_t button_isr(int irq, void *dev_id)
{
    int value = gpio_get_value(btn_gpio); // 0 when pressed (active low)

    pr_info("[mydriver] Button ISR: value = %d\n", value);

    gpio_set_value(led_gpio, value ? 1 : 0); // LED ON when button pressed (0)

    return IRQ_HANDLED;
}

static int mydriver_probe(struct platform_device *pdev)
{
    int ret;

    pr_info("[mydriver] === PROBE START ===\n");

    // Get LED GPIO
    led_gpio = of_get_named_gpio(pdev->dev.of_node, "led-gpio", 0);
    if (led_gpio < 0) {
        pr_err("[mydriver] Failed to get led-gpio\n");
        return -EINVAL;
    }

    // Get Button GPIO
    btn_gpio = of_get_named_gpio(pdev->dev.of_node, "button-gpio", 0);
    if (btn_gpio < 0) {
        pr_err("[mydriver] Failed to get button-gpio\n");
        return -EINVAL;
    }

    pr_info("[mydriver] LED GPIO: %d, Button GPIO: %d\n", led_gpio, btn_gpio);

    // Request and setup LED GPIO
    ret = gpio_request(led_gpio, "led-gpio");
    if (ret) {
        pr_err("[mydriver] Failed to request led-gpio\n");
        return ret;
    }

    ret = gpio_direction_output(led_gpio, 0); // Initially off
    if (ret) {
        pr_err("[mydriver] Failed to set led-gpio direction\n");
        gpio_free(led_gpio);
        return ret;
    }

    // Request and setup Button GPIO
    ret = gpio_request(btn_gpio, "button-gpio");
    if (ret) {
        pr_err("[mydriver] Failed to request button-gpio\n");
        gpio_free(led_gpio);
        return ret;
    }

    ret = gpio_direction_input(btn_gpio);
    if (ret) {
        pr_err("[mydriver] Failed to set button-gpio direction\n");
        gpio_free(led_gpio);
        gpio_free(btn_gpio);
        return ret;
    }

    // Get IRQ and request it
    irq_number = gpio_to_irq(btn_gpio);
    if (irq_number < 0) {
        pr_err("[mydriver] Failed to get IRQ number\n");
        gpio_free(led_gpio);
        gpio_free(btn_gpio);
        return irq_number;
    }

    ret = request_irq(irq_number, button_isr,
                      IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
                      "mydriver_button_irq", NULL);
    if (ret) {
        pr_err("[mydriver] Failed to request IRQ\n");
        gpio_free(led_gpio);
        gpio_free(btn_gpio);
        return ret;
    }

    pr_info("[mydriver] Driver loaded successfully\n");
    pr_info("[mydriver] === PROBE END ===\n");

    return 0;
}

static int mydriver_remove(struct platform_device *pdev)
{
    pr_info("[mydriver] === REMOVE START ===\n");

    if (irq_number >= 0) {
        free_irq(irq_number, NULL);
        pr_info("[mydriver] IRQ freed\n");
    }

    if (btn_gpio >= 0)
        gpio_free(btn_gpio);

    if (led_gpio >= 0) {
        gpio_set_value(led_gpio, 0); // Turn off LED
        gpio_free(led_gpio);
    }

    pr_info("[mydriver] Driver unloaded\n");
    pr_info("[mydriver] === REMOVE END ===\n");
    return 0;
}

// Device Tree match
static const struct of_device_id mydriver_dt_ids[] = {
    { .compatible = "my,gpio-test", },
    { }
};
MODULE_DEVICE_TABLE(of, mydriver_dt_ids);

// Platform driver
static struct platform_driver mydriver_platform_driver = {
    .probe = mydriver_probe,
    .remove = mydriver_remove,
    .driver = {
        .name = "mydriver",
        .of_match_table = mydriver_dt_ids,
    },
};

module_platform_driver(mydriver_platform_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("wings");
MODULE_DESCRIPTION("Simple GPIO driver: LED on when button pressed");

