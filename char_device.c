#include <linux/types.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/slab.h>    // -> kmalloc
#include <linux/timer.h>
#include <linux/err.h>
#include <linux/hrtimer.h>
#include <linux/sched.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <mach/platform.h>

#include "char_device.h"
#include "utils.h"
#include "8x8font.h"

MODULE_LICENSE("Dual BSD/GPL");

// The device
static struct cdev char_device;

// Memory device - The device manipulated by file_operations
static struct mem_dev *mem_dev_p;

/****************************************************************************/
/* Timer variables block                                                    */
/****************************************************************************/
static enum hrtimer_restart function_timer(struct hrtimer *);
static struct hrtimer htimer;
static ktime_t kt_periode;

static struct timer_list s_BlinkTimer;

static int row[8] = {4, 25, 24, 23, 22, 27, 18, 17};
static int lin[8] = {2, 3, 8, 7, 10, 9, 11, 14};
static int lin_msk, row_msk;
static int _lin = 0;
static char ch = 'X';

static void BlinkTimerHandler(unsigned long unused)
{

    mod_timer (&s_BlinkTimer, jiffies + 1);
}

static enum hrtimer_restart function_timer(struct hrtimer * unused)
{
    int k, msk, clr=0, st=0;
    msk = (font[ch]>>(_lin*8)) & 0xff;
    for (k=7; ~k; --k, msk>>=1) {
        int p = 1 << row[7-k];
        if (msk & 1) st |= p;
        else clr |= p;
    }
    digitalWrite ((lin_msk^(1<<lin[_lin])) | st, HIGH);
    digitalWrite ((1<<lin[_lin]) | clr, LOW);
    _lin = (_lin + 1) & 7;
    hrtimer_forward_now(& htimer, kt_periode);
    return HRTIMER_RESTART;
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

ssize_t char_device_write(struct file *filp, const char __user *buf, size_t size, loff_t *ppos) {

    if (!filp || !buf || !ppos) {
        printk(KERN_ALERT "char_device_write : Bad parameters!");
        return -ENODEV;
    }

    // check buffer size
    struct mem_dev *p_dev = filp->private_data;
    if (size > p_dev->size) {
        printk(KERN_ALERT "writing too much data at the same time!");
        goto out;
    }

    // Now copy data to kernel space
    if (copy_from_user(p_dev->data, buf, size)) {
        goto out;
    } else {
        ch = p_dev->data[0];
        printk("ch is now %c\n", ch);
        return size;
    }

out:
    printk(KERN_ALERT "copy failed");
    return -EFAULT;
}

// File operator - It set up the connection between the dirver's operators
// and device number
static const struct file_operations char_device_ops = {
    .owner = THIS_MODULE,
    //.llseek = char_device_llseek,
    //.read = char_device_read,
    .write = char_device_write,
    .open = char_device_open,
    .release = char_device_release,
};

static int char_device_init(void) {
    int i;
    printk(KERN_ALERT "Init!");
    dev_t device_num = MKDEV(MAJOR_DEVICE_NUM, 0);
    printk(KERN_ALERT "Device number is %d\n", device_num);
    printk(KERN_ALERT "Major number is %d\n", MAJOR(device_num));
    printk(KERN_ALERT "Minor number is %d\n", MINOR(device_num));
    /* ------------ GPIO BEGIN------------- */
    _pGpioRegisters = (struct GpioRegisters *)__io_address(GPIO_BASE);
    lin_msk = row_msk = 0;
    for (i=0; i<8; ++i) {
        pinMode(lin[i], OUTPUT);
        pinMode(row[i], OUTPUT);
        lin_msk |= 1 << lin[i];
        row_msk |= 1 << row[i];
    }

    /* ------------- GPIO END------------- */

    /* ------------- HRTIMER BEGIN ----------- */

    kt_periode = ktime_set(0, 100); //seconds,nanoseconds
    hrtimer_init (& htimer, CLOCK_REALTIME, HRTIMER_MODE_REL);
    htimer.function = function_timer;
    hrtimer_start(& htimer, kt_periode, HRTIMER_MODE_REL);

    /* ------------- HRTIMER END ------------- */

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

    for (i=0; i < MEMDEV_NUM; ++i) {
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
    hrtimer_cancel(& htimer);

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
