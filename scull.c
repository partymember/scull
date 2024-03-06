#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>

#define MODULE_NAME "scull"
#define DEV_COUNT 4
#define FIRST_MINOR 0


MODULE_LICENSE("Dual BSD/GPL");

dev_t dev;

static int __init init(void){
    int r=0;
    printk(KERN_ALERT "init\n");
    r = alloc_chrdev_region(&dev, FIRST_MINOR, DEV_COUNT, MODULE_NAME);
    if(r<0){
        printk(KERN_WARNING "Cannot allocate char device major");
	return r;
    }
    printk(KERN_ALERT "Got major: %d, minor: %d\n", MAJOR(dev), MINOR(dev));
    return 0;
    
}

static void __exit cleanup(void){
    printk(KERN_ALERT "exit\n");
    unregister_chrdev_region(dev, DEV_COUNT);
}


module_init(init);
module_exit(cleanup);
