#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>
#include "scull.h"


dev_t first_dev;
int scull_major;
int scull_minor = 0;




MODULE_LICENSE("GPL");





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
    return 0;
}
static void __exit scull_exit(void){
printk("Releasing scull...");
unregister_chrdev_region(first_dev, DEV_COUNT);

printk(KERN_ALERT "Close scull");
}
/*

struct file_operatrions scull_fops ={
.owner =    THIS_MODULE,
.llseek =   scull_llseek,
.read =     scull_read,
.write =    scull_write,
.ioctl =    scull_ioctl,
.open =     scull_open,
.release =  scull_release,
}*/

module_init(scull_init);
module_exit(scull_exit);
