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

#define DRIVER_AUTHOR   "Tran Thanh Vu <1612823@student.hcmus.edu.com>"
#define DRIVER_DESC     "Generate a random number"
#define DRIVER_VERSION  "0.1"

static dev_t devNum;            // device number
static struct class *devClass;  // device class
static struct cdev *vcdev;      // mô tả cấu trúc character device 

static int driver_open(struct inode *inode, struct file *filp) {
    printk(KERN_INFO "Driver: open()\n");
    return 0;
}

static int driver_close(struct inode *inode, struct file *filp) {
    printk(KERN_INFO "Driver: close()\n");
    return 0;
}

static ssize_t driver_read(struct file *filp, char __user *user_buf, size_t len, loff_t *off) {
    printk(KERN_INFO "Driver: read()\n");
    
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
    /* Đăng kí device number */ 
    if (alloc_chrdev_region(&devNum, 0, 1, "randomDevice") < 0) {
        return -1;
    }

    printk(KERN_INFO "Registered device number successfully. ");
    printk(KERN_INFO "<Major, Minor>: <%d, %d> \n", MAJOR(devNum), MINOR(devNum));

    /* Tao device file */
    // Tạo device class
    if ((devClass = class_create(THIS_MODULE, "class_random_device")) == NULL) {
        unregister_chrdev_region(devNum, 1);
        return -1;
    }

    printk(KERN_INFO "Create device class successfully \n");
    
    //Tạo thông tin của device
    if ((device_create(devClass, NULL, devNum, NULL, "random_device")) == NULL) {
        class_destroy(devClass);
        unregister_chrdev_region(devNum, 1);
        return -1;
    }

    printk(KERN_INFO "Create device file successfully \n");

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

    printk(KERN_INFO "Random number generator registered");
    
    return 0;
}
    
// Hàm kết thúc module 
static void __exit exit_randomNum(void) {
    // Hủy thao tác với device file
    cdev_del(vcdev);

    // Xóa device file 
    device_destroy(devClass, devNum);
    class_destroy(devClass);

    // Hủy device number 
    unregister_chrdev_region(devNum, 1);

    printk(KERN_INFO "Random number generator unregistered");

}

module_init(init_randomNum);
module_exit(exit_randomNum);

MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_VERSION(DRIVER_VERSION);
MODULE_SUPPORTED_DEVICE("Random Device");
