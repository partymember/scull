#include <linux/init.h>
#include <linux/module.h>

MODULE_LICENSE("Dual BSD/GPL");

static int init(void){
    printk("Hello");
    return 0;
}

static void exit(void){
    printk("exit");
}


module_init(init);
module_exit(exit);
