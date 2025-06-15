#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/gpio/consumer.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "oxcode"
#define CLASS_NAME "oxcode_class"
#define GPIO_0_LABEL "GPIO17"
#define GPIO_1_LABEL "GPIO27"

static dev_t dev;
static struct cdev c_dev;
static struct class *cl;
static char curr_signal = '0';
static struct gpio_desc *gpio0_desc, *gpio1_desc;

static int my_open(struct inode *i, struct file *f)
{
    printk(KERN_INFO "0xC0DE: open()\n");
    return 0;
}

static int my_close(struct inode *i, struct file *f)
{
    printk(KERN_INFO "0xC0DE: close()\n");
    return 0;
}

static ssize_t my_read(struct file *f, char __user *buf, size_t len, loff_t *off)
{
    char kbuf[1];
    int value;

    if (curr_signal == '0') {
        value = gpiod_get_value(gpio0_desc);
    } else if (curr_signal == '1') {
        value = gpiod_get_value(gpio1_desc);
    } else {
        return -EINVAL;
    }

    kbuf[0] = value ? '1' : '0';

    if (copy_to_user(buf, kbuf, 1))
        return -EINVAL;

    return 1;
}

static ssize_t my_write(struct file *f, const char __user *buf, size_t len, loff_t *off)
{
    char kbuf;

    if (len != 1)
        return -EINVAL;

    if (copy_from_user(&kbuf, buf, 1))
        return -EFAULT;

    if (kbuf != '0' && kbuf != '1')
        return -EINVAL;

    curr_signal = kbuf;
    return len;
}

static struct file_operations dev_fops = {
    .owner = THIS_MODULE,
    .open = my_open,
    .release = my_close,
    .read = my_read,
    .write = my_write
};

static int __init oxcode_init(void)
{
    int ret;
    dev_t dev_no;
    struct device *dev_ret;

    if ((ret = alloc_chrdev_region(&dev_no, 0, 1, DEVICE_NAME))) {
        pr_err("0xC0DE: Failed to allocate major number\n");
        return ret;
    }
    dev = MKDEV(MAJOR(dev_no), 0);

    cdev_init(&c_dev, &dev_fops);
    if ((ret = cdev_add(&c_dev, dev, 1))) {
        pr_err("0xC0DE: Failed to add character device\n");
        goto fail_cdev;
    }

    cl = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(cl)) {
        ret = PTR_ERR(cl);
        pr_err("0xC0DE: Failed to create device class\n");
        goto fail_class;
    }

    dev_ret = device_create(cl, NULL, dev, NULL, DEVICE_NAME);
    if (IS_ERR(dev_ret)) {
        ret = PTR_ERR(dev_ret);
        pr_err("0xC0DE: Failed to create device\n");
        goto fail_device;
    }

    gpio0_desc = gpiod_get_index(NULL, GPIO_0_LABEL, 0, GPIOD_IN);
    if (IS_ERR(gpio0_desc)) {
        ret = PTR_ERR(gpio0_desc);
        pr_err("0xC0DE: Failed to get GPIO 0 descriptor\n");
        goto fail_gpio0;
    }

    gpio1_desc = gpiod_get_index(NULL, GPIO_1_LABEL, 1, GPIOD_IN);
    if (IS_ERR(gpio1_desc)) {
        ret = PTR_ERR(gpio1_desc);
        pr_err("0xC0DE: Failed to get GPIO 1 descriptor\n");
        goto fail_gpio1;
    }

    pr_info("0xC0DE: Module initialized successfully\n");
    return 0;

fail_gpio1:
    gpiod_put(gpio0_desc);
fail_gpio0:
    device_destroy(cl, dev);
fail_device:
    class_destroy(cl);
fail_class:
    cdev_del(&c_dev);
fail_cdev:
    unregister_chrdev_region(dev, 1);
    return ret;
}

static void __exit oxcode_exit(void)
{
    gpiod_put(gpio0_desc);
    gpiod_put(gpio1_desc);
    device_destroy(cl, dev);
    class_destroy(cl);
    cdev_del(&c_dev);
    unregister_chrdev_region(dev, 1);
    pr_info("0xC0DE: Module unloaded successfully\n");
}

module_init(oxcode_init);
module_exit(oxcode_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Grupo 0XC0DE - Sistemas de computacion");
MODULE_DESCRIPTION("Driver para lectura de GPIO con gpiod");
