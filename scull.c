#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/cdev.h>
//#include "scull.h"

#define DEV_COUNT 1
#define DRIVER_NAME "scull"

dev_t first_dev;

int scull_major;
int scull_minor = 0;
/*
struct scull_dev {
    struct scull_qset *data;
    int quantum;
    int qset;
    unsigned long size;
    unsigned int access_key;
    struct semaphore sem;
    struct cdev cdev;
}
*/
struct cdev cdev;




static int  scull_open(struct inode* i, struct file* f){
    printk(KERN_ALERT "scull_open()");
    return 0;
}
static ssize_t  scull_read(struct file *f, char __user *buf, size_t len, loff_t *off){
    printk(KERN_ALERT "scull_read()");
    return 0;
}
static ssize_t  scull_write(struct file *f, const char __user *buf, size_t len, loff_t *off){
    printk(KERN_ALERT "scull_write()");
    return len;
}
static int scull_release(struct inode* i, struct file* f){
    printk(KERN_ALERT "scull_release()");
    return 0;
}


struct file_operations scull_fops = {
.owner =    THIS_MODULE,
//.llseek =   scull_llseek,
.read =     scull_read,
.write =    scull_write,
//.ioctl =    scull_ioctl,
.open =     scull_open,
.release =  scull_release,
};

static int __init scull_init(void)
{
    int r;
    printk("Opening scull...");
    r = alloc_chrdev_region(&first_dev,scull_minor, DEV_COUNT, DRIVER_NAME);
    if(r){
        printk("Opening scull error: alloc_chrdev_region");
        return r;
    }
    scull_major = MAJOR(first_dev);


    printk(KERN_ALERT "Open scull MAJOR: %d", scull_major);
    cdev_init(&cdev, &scull_fops);
    cdev.owner = THIS_MODULE;

    r = cdev_add(&cdev,first_dev,1);
    if(r){
        printk(KERN_ALERT "Error %d adding scull device",r);
        unregister_chrdev_region(first_dev, DEV_COUNT);
        return r;
    }
    printk("Done");


    return 0;
}
static void __exit scull_exit(void){
printk(KERN_ALERT "Releasing scull...");
cdev_del(&cdev);
unregister_chrdev_region(first_dev, DEV_COUNT);

printk(KERN_ALERT "Close scull");
printk(KERN_ALERT "Close sculld");
}




module_init(scull_init);
module_exit(scull_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Adam Mikolajczak");
MODULE_DESCRIPTION("Scull driver");
