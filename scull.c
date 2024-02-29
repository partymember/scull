#include <linux/init.h>
#include <linux/module.h>

MODULE_LICENSE("Dual BSD/GPL");

static int __init init(void){
    printk(KERN_ALERT "init\n");
    return 0;
}

static void __exit cleanup(void){
    printk(KERN_ALERT "exit\n");
}


module_init(init);
module_exit(cleanup);
