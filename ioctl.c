#include <linux/module.h>       // Core header for kernel modules
#include <linux/fs.h>           // File operations: open, read, write, etc.
#include <linux/uaccess.h>      // Functions for user access: copy_to_user, etc.
#include <linux/cdev.h>         // Character device structures
#include <linux/device.h>       // Device creation: class, device_create
#include <linux/poll.h>         // Support for poll/select system calls
#include <linux/mutex.h>        // Kernel mutex support
#include <linux/ioctl.h>        // IOCTL macros and definitions

#define DEVICE_NAME "gpio_ctrl"
#define CLASS_NAME  "gpio_class"

#define GPIO_CTRL_MAGIC   'G'
#define GPIO_GET_STATUS   _IOR(GPIO_CTRL_MAGIC, 0, int)  // Read LED & Button status
#define GPIO_TOGGLE_LED   _IO(GPIO_CTRL_MAGIC, 1)        // Toggle LED command

static dev_t dev_num;
static struct cdev gpio_cdev;
static struct class *gpio_class = NULL;
static struct device *gpio_device;

static DEFINE_MUTEX(gpio_mutex);              // Mutex for synchronization
static DECLARE_WAIT_QUEUE_HEAD(wq);           // Wait queue for poll
static bool data_ready = false;               // Flag for poll readiness

// External GPIO functions implemented in separate modules
extern void gpio_led_toggle(void);
extern int get_led_status(void);
extern int get_button_status(void);

/**
 * gpio_ctrl_open - Open the GPIO control device
 * @inode: Pointer to inode structure
 * @file: Pointer to file structure
 *
 * Called when the device is opened from user space.
 *
 * Return: 0 on success.
 */
static int gpio_ctrl_open(struct inode *inode, struct file *file)
{
    pr_info("gpio_ctrl: Device opened\n");
    return 0;
}

/**
 * gpio_ctrl_release - Close the GPIO control device
 * @inode: Pointer to inode structure
 * @file: Pointer to file structure
 *
 * Called when the device is closed from user space.
 *
 * Return: 0 on success.
 */
static int gpio_ctrl_release(struct inode *inode, struct file *file)
{
    pr_info("gpio_ctrl: Device closed\n");
    return 0;
}

/**
 * gpio_ctrl_write - Write command to device (e.g., "toggle")
 * @file: File pointer
 * @buf: User-space buffer
 * @count: Number of bytes written
 * @ppos: File position pointer
 *
 * Parses user command. If it is "toggle", toggles the LED and signals poll.
 *
 * Return: Number of bytes written on success, or -EFAULT/-EINVAL on error.
 */
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

/**
 * gpio_ctrl_read - Read LED and button status as a formatted string
 * @file: File pointer
 * @buf: User-space buffer
 * @count: Number of bytes to read
 * @ppos: File position pointer
 *
 * Returns: "LED: ON | Button: PRESSED" or similar text.
 *
 * Return: Number of bytes read or -EFAULT if copy_to_user fails.
 */
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

/**
 * gpio_ctrl_ioctl - Handle IOCTL commands from user space
 * @file: File pointer
 * @cmd: IOCTL command
 * @arg: Argument from user space
 *
 * Supported commands:
 * - GPIO_GET_STATUS: Return combined LED + Button status (bit 1 = LED, bit 0 = button)
 * - GPIO_TOGGLE_LED: Toggle the LED state
 *
 * Return: 0 on success, -EFAULT or -EINVAL on error.
 */
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

/**
 * gpio_ctrl_poll - Support for poll/select system calls
 * @file: File pointer
 * @wait: Poll table structure
 *
 * Allows user-space processes to wait for LED toggle events.
 *
 * Return: POLLIN | POLLRDNORM if data is ready, 0 otherwise.
 */
static __poll_t gpio_ctrl_poll(struct file *file, struct poll_table_struct *wait)
{
    poll_wait(file, &wq, wait);
    if (data_ready) {
        data_ready = false;
        return POLLIN | POLLRDNORM;
    }
    return 0;
}

// File operations structure mapping system calls to handlers
static const struct file_operations gpio_fops = {
    .owner          = THIS_MODULE,
    .open           = gpio_ctrl_open,
    .release        = gpio_ctrl_release,
    .read           = gpio_ctrl_read,
    .write          = gpio_ctrl_write,
    .unlocked_ioctl = gpio_ctrl_ioctl,
    .poll           = gpio_ctrl_poll,
};

/**
 * gpio_ctrl_init - Module initialization function
 *
 * Allocates a character device number, initializes and registers
 * the char device, creates sysfs class and device file.
 *
 * Return: 0 on success, or negative error code on failure.
 */
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

/**
 * gpio_ctrl_exit - Module exit function
 *
 * Cleans up the character device, class, and device file.
 * Releases allocated resources and logs the unload event.
 */
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
