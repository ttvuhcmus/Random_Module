#include <linux/module.h> 
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/random.h>

#define DRIVER_AUTHOR   "1612808_1612818_1612823"
#define DRIVER_DESC     "Random Number Generator"
#define DRIVER_VERSION  "0.1"

static dev_t devNum;            // device number
static struct class *devClass;  // device class
static struct cdev *vcdev;      // character device structure 

static int driver_open(struct inode *inode, struct file *filp) {
    printk(KERN_INFO "Module has been opened \n");
    return 0;
}

static int driver_close(struct inode *inode, struct file *filp) {
    printk(KERN_INFO "Module successfully closed \n");
    return 0;
}

static ssize_t driver_read(struct file *filp, char __user *user_buf, size_t len, loff_t *off) {
    int num;
    get_random_bytes(&num, sizeof(num));

    if (copy_to_user(user_buf, &num, sizeof(num)) != 0)
        return -EINVAL;

    return 0;
}

// mô tả cấu trúc file operations
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = driver_open,
    .release = driver_close,
    .read = driver_read,
};

// Hàm khởi tạo module 
static int __init init_randomNum(void) {
	printk(KERN_INFO "Initializing modules ... ");

    /* Tạo device number */ 
    if (alloc_chrdev_region(&devNum, 0, 1, "randomDevice") < 0) {
        return -1;
    }

    printk(KERN_INFO "<Major, Minor>: <%d, %d> \n", MAJOR(devNum), MINOR(devNum));

    /* Tao device file */
    // Tạo device class
    if ((devClass = class_create(THIS_MODULE, "class_random_device")) == NULL) {
        unregister_chrdev_region(devNum, 1);
        return -1;
    }

    printk(KERN_INFO "Device class created successfully \n");
    
    // Tạo device file
    if ((device_create(devClass, NULL, devNum, NULL, "random_device")) == NULL) {
        class_destroy(devClass);
        unregister_chrdev_region(devNum, 1);
        return -1;
    }

    printk(KERN_INFO "Device file created successfully \n");

	/* Đăng kí thao tác với device file */
	// Cấp phát thao tác
	if ((vcdev = cdev_alloc()) == NULL) {
        device_destroy(devClass, devNum);
        class_destroy(devClass);
        unregister_chrdev_region(devNum, 1);
        return -1;
    }

    // Khởi tạo thao tác
    cdev_init(vcdev, &fops);

    // Đăng kí thao tác
    if (cdev_add(vcdev, devNum, 1) < 0) {
        device_destroy(devClass, devNum);
        class_destroy(devClass);
        unregister_chrdev_region(devNum, 1);
        return -1;
    }

    printk(KERN_INFO "Module has been successfully initialized !!!");
    
    return 0;
}
    
// Hàm kết thúc module 
static void __exit exit_randomNum(void) {
    cdev_del(vcdev);						// Hủy thao tác với device file
    device_destroy(devClass, devNum);		// Hủy device file 
    class_destroy(devClass);				// Hủy device class
    unregister_chrdev_region(devNum, 1);	// Hủy device number 

    printk(KERN_INFO "Goodbye !!!");
}

module_init(init_randomNum);
module_exit(exit_randomNum);

MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_VERSION(DRIVER_VERSION);
MODULE_SUPPORTED_DEVICE("Random Device");
