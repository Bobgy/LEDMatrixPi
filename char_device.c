#include <linux/types.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/slab.h>    // -> kmalloc
#include <linux/timer.h>
#include <linux/err.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <mach/platform.h>

#include "char_device.h"
#include "utils.h"

MODULE_LICENSE("Dual BSD/GPL");

// The device
static struct cdev char_device;

// Memory device - The device manipulated by file_operations
static struct mem_dev *mem_dev_p;

static struct timer_list s_BlinkTimer;
static int s_BlinkPeriod = 1000;
static const int LedGpioPin = 18;

static void BlinkTimerHandler(unsigned long unused)
{
    static bool on = false;
    on = !on;
    digitalWrite(LED, on);
    mod_timer(&s_BlinkTimer,
              jiffies + msecs_to_jiffies(s_BlinkPeriod));
}

// open this device
int char_device_open(struct inode *inode, struct file *filp) {
    unsigned int minor = iminor(inode);

    if (minor >= MAX_MINOR_NUM) {
        printk(KERN_ALERT "Bad minor number!");
        return -ENODEV;
    }

    struct mem_dev *p = &mem_dev_p[minor];

    // let filp->private_data points to mem_dev
    filp->private_data = p;

    return 0;
}

int char_device_release(struct inode *inode, struct file *filp) {
    return 0;
}

ssize_t char_device_read(struct file *filp, char __user *buf, size_t size, loff_t *ppos) {
    if (!filp || !buf || !ppos) {
        printk(KERN_ALERT "char_device_read : Bad parameters!");
        return -ENODEV;
    }

    // stop the LED
    digitalWrite(LED, LOW);

    // check buffer size
    struct mem_dev *p_dev = filp->private_data;
    if (size > p_dev->size)
        goto out;
    if (*ppos + size > p_dev->size)
        size = p_dev->size - *ppos;

    // Now copy data to user space
    if (copy_to_user(buf, p_dev->data + *ppos, size)) {
        goto out;
    } else {
        *ppos += size;
        return size;
    }

out:
    return -EFAULT;
}

ssize_t char_device_write(struct file *filp, const char __user *buf, size_t size, loff_t *ppos) {
    if (!filp || !buf || !ppos) {
        printk(KERN_ALERT "char_device_write : Bad parameters!");
        return -ENODEV;
    }

    // light the LED
    digitalWrite(LED, HIGH);

    // check buffer size
    struct mem_dev *p_dev = filp->private_data;
    if (size > p_dev->size)
        goto out;
    if (*ppos + size > p_dev->size)
        size = p_dev->size - *ppos;

    // Now copy data to kernel space
    if (copy_from_user(p_dev->data + *ppos, buf, size)) {
        goto out;
    } else {
        *ppos + size;
        return size;
    }

out:
    return -EFAULT;
}

// File operator - It set up the connection between the dirver's operators
// and device number
static const struct file_operations char_device_ops = {
    .owner = THIS_MODULE,
    //.llseek = char_device_llseek,
    .read = char_device_read,
    .write = char_device_write,
    .open = char_device_open,
    .release = char_device_release,
};



static int char_device_init(void) {
    printk(KERN_ALERT "Init!");
    dev_t device_num = MKDEV(MAJOR_DEVICE_NUM, 0);
    printk(KERN_ALERT "Device number is %d\n", device_num);
    printk(KERN_ALERT "Major number is %d\n", MAJOR(device_num));
    printk(KERN_ALERT "Minor number is %d\n", MINOR(device_num));

    /* ------------ GPIO BEGIN------------- */
    _pGpioRegisters = (struct GpioRegisters *)__io_address(GPIO_BASE);
    pinMode(LED, OUTPUT);

    setup_timer(&s_BlinkTimer, BlinkTimerHandler, 0);
    int result = mod_timer(&s_BlinkTimer,
                       jiffies + msecs_to_jiffies(s_BlinkPeriod));
    if (!result) {
        printk(KERN_ALERT "Failed adding timer\n");
        goto register_fail;
    }
    /* ------------- GPIO END------------- */

    // Allocate/Register device number, now it appreas in /proc/device
    if (register_chrdev_region(device_num, MAX_MINOR_NUM, "char_device")) {
        printk(KERN_ALERT "Failed to allocate device number");
        goto register_fail;
    }

    // Init a deivce, establish the connection between device and operations
    cdev_init(&char_device, &char_device_ops);

    // Let kernel know the existence of this device
    int err = cdev_add(&char_device, device_num, MAX_MINOR_NUM);
    if (err) {
        printk(KERN_ALERT "cdev_add fail!");
        goto fail_malloc;
    }

    // Allocate memory for device
    mem_dev_p = kmalloc(sizeof(struct mem_dev) * MEMDEV_NUM, GFP_KERNEL);
    if (!mem_dev_p) {
        printk(KERN_ALERT "mem_dev_p malloc fail!");
        goto fail_malloc;
    }

    int i = 0;
    for (; i < MEMDEV_NUM; ++i) {
        mem_dev_p[i].data = kmalloc(MEMDEV_SIZE, GFP_KERNEL);
        if (!mem_dev_p[i].data) {
            printk(KERN_ALERT "mem_dev_p[i].data malloc fail!");
            goto fail_data_malloc;
        }

        mem_dev_p[i].size = MEMDEV_SIZE;
        memset(mem_dev_p[i].data, 0, mem_dev_p[i].size);
    }

    return 0;

fail_data_malloc:
    kfree(mem_dev_p);
fail_malloc:
    unregister_chrdev_region(MKDEV(MAJOR_DEVICE_NUM, 0), MAX_MINOR_NUM);
register_fail:
    return 0;
}

static void char_device_exit(void) {
    printk(KERN_ALERT "Goodbye!");

    // GPIO
    del_timer(&s_BlinkTimer);

    // free memory device
    int i = 0;
    for (; i < MEMDEV_NUM; ++i) {
        if (mem_dev_p && mem_dev_p[i].data)
            kfree(mem_dev_p[i].data);
    }

    if (mem_dev_p)
        kfree(mem_dev_p);

    cdev_del(&char_device);
    unregister_chrdev_region(MKDEV(MAJOR_DEVICE_NUM, 0), MAX_MINOR_NUM);
}

module_init(char_device_init);
module_exit(char_device_exit);
