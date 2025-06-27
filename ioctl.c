#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/poll.h>
#include <linux/mutex.h>
#include <linux/ioctl.h>

#define DEVICE_NAME "gpio_ctrl"
#define CLASS_NAME  "gpio_class"

#define GPIO_CTRL_MAGIC   'G'
#define GPIO_GET_STATUS   _IOR(GPIO_CTRL_MAGIC, 0, int)
#define GPIO_TOGGLE_LED   _IO(GPIO_CTRL_MAGIC, 1)

static dev_t dev_num;
static struct cdev gpio_cdev;
static struct class *gpio_class = NULL;
static struct device *gpio_device;

static DEFINE_MUTEX(gpio_mutex);
static DECLARE_WAIT_QUEUE_HEAD(wq);
static bool data_ready = false;

extern void gpio_led_toggle(void);
extern int get_led_status(void);
extern int get_button_status(void);

// ----------- File operations -----------

static int gpio_ctrl_open(struct inode *inode, struct file *file)
{
    pr_info("gpio_ctrl: Device opened\n");
    return 0;
}

static int gpio_ctrl_release(struct inode *inode, struct file *file)
{
    pr_info("gpio_ctrl: Device closed\n");
    return 0;
}

static ssize_t gpio_ctrl_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    char cmd[16] = {0};

    if (copy_from_user(cmd, buf, min(count, sizeof(cmd) - 1)))
        return -EFAULT;

    pr_info("gpio_ctrl: Received write command: %s\n", cmd);

    if (strncmp(cmd, "toggle", 6) == 0) {
        gpio_led_toggle();
        data_ready = true;
        wake_up_interruptible(&wq);
        return count;
    }

    pr_warn("gpio_ctrl: Invalid write command\n");
    return -EINVAL;
}

static ssize_t gpio_ctrl_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    char status[64];
    int len;

    int led = get_led_status();
    int button = get_button_status();

    len = snprintf(status, sizeof(status), "LED: %s | Button: %s\n",
                   led ? "ON" : "OFF",
                   button ? "PRESSED" : "RELEASED");

    if (*ppos > 0 || count < len)
        return 0;

    if (copy_to_user(buf, status, len))
        return -EFAULT;

    *ppos += len;
    return len;
}

static long gpio_ctrl_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    switch (cmd) {
    case GPIO_GET_STATUS: {
        int status = (get_led_status() << 1) | get_button_status();
        if (copy_to_user((int __user *)arg, &status, sizeof(status)))
            return -EFAULT;
        pr_info("gpio_ctrl: IOCTL - returned status LED=%d, BTN=%d\n",
                get_led_status(), get_button_status());
        return 0;
    }
    case GPIO_TOGGLE_LED:
        gpio_led_toggle();
        pr_info("gpio_ctrl: IOCTL - toggled LED\n");
        return 0;
    default:
        pr_warn("gpio_ctrl: IOCTL - invalid command\n");
        return -EINVAL;
    }
}

static __poll_t gpio_ctrl_poll(struct file *file, struct poll_table_struct *wait)
{
    poll_wait(file, &wq, wait);
    if (data_ready) {
        data_ready = false;
        return POLLIN | POLLRDNORM;
    }
    return 0;
}

// ----------- File operation structure -----------

static const struct file_operations gpio_fops = {
    .owner          = THIS_MODULE,
    .open           = gpio_ctrl_open,
    .release        = gpio_ctrl_release,
    .read           = gpio_ctrl_read,
    .write          = gpio_ctrl_write,
    .unlocked_ioctl = gpio_ctrl_ioctl,
    .poll           = gpio_ctrl_poll,
};

// ----------- Init / Exit -----------

static int __init gpio_ctrl_init(void)
{
    int ret;

    ret = alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
    if (ret) {
        pr_err("gpio_ctrl: Failed to allocate chrdev region\n");
        return ret;
    }

    cdev_init(&gpio_cdev, &gpio_fops);
    gpio_cdev.owner = THIS_MODULE;

    ret = cdev_add(&gpio_cdev, dev_num, 1);
    if (ret) {
        pr_err("gpio_ctrl: Failed to add cdev\n");
        unregister_chrdev_region(dev_num, 1);
        return ret;
    }

    gpio_class = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(gpio_class)) {
        pr_err("gpio_ctrl: Failed to create class\n");
        cdev_del(&gpio_cdev);
        unregister_chrdev_region(dev_num, 1);
        return PTR_ERR(gpio_class);
    }

    gpio_device = device_create(gpio_class, NULL, dev_num, NULL, DEVICE_NAME);
    if (IS_ERR(gpio_device)) {
        pr_err("gpio_ctrl: Failed to create device\n");
        class_destroy(gpio_class);
        cdev_del(&gpio_cdev);
        unregister_chrdev_region(dev_num, 1);
        return PTR_ERR(gpio_device);
    }

    mutex_init(&gpio_mutex);

    pr_info("gpio_ctrl: Registered with major %d\n", MAJOR(dev_num));
    pr_info("gpio_ctrl: Device initialized successfully\n");

    return 0;
}

static void __exit gpio_ctrl_exit(void)
{
    device_destroy(gpio_class, dev_num);
    class_destroy(gpio_class);
    cdev_del(&gpio_cdev);
    unregister_chrdev_region(dev_num, 1);
    pr_info("gpio_ctrl: Module unloaded\n");
}

module_init(gpio_ctrl_init);
module_exit(gpio_ctrl_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Wings Mon");
MODULE_DESCRIPTION("GPIO control driver via ioctl/read/write/poll");
