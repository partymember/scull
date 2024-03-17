#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/slab.h>

#define MODULE_NAME "scull"
#define DEV_COUNT 4
#define FIRST_MINOR 0


MODULE_LICENSE("Dual BSD/GPL");

int scull_major = 0;
int scull_minor = FIRST_MINOR;

struct scull_qset = {
  void **data;
  struct scull_qset *next;
}


struct scull_dev = {
  struct scull_qset *data;
  int quantum;
  int qset;
  unsigned long size;
  unsigned int access_key;
  struct semaphore sem;
  struct cdev cdev;
};

dev_t dev;

struct file_operations fops = {
  .owner = THIS_MODULE,
  .open = scull_open,
  .release = scull_release,
  .read = scull_read,
  .write = scull_write,
  .llseek = scull_llseek
  .ioctl = scull_ioctl,
};

static void scull_setup_cdev(struct scull_dev *dev, int index){
  int err=0;
  int devno = MKDEV(scull_major, scull_minor+index);
  cdev_init(&dev->cdev, &fops);
  dev->cdev.owner=THIS_MODULE;
  dev->cdev.fops=&fops;
  err=cdev_add(&dev->cdev, devno, index);
  if(err){
    printf(KERN_NOTICE "Err %d!Cannot add scull%d", err, index);
  }
}

int scull_open(struct inode *inode, struct file *filp){
  struct scull_dev *dev;
  dev=container_of(inode->i_cdev, struct scull_dev, cdev);
  filp->private_data=dev;
  if((filp->f_flags & O_ACCMODE) == O_WRONLY)){
    scull_trim(dev):
  }
  return 0;
}

int scull_release(struct inode *inode, struct file *filp){
  return 0;
}




static int __init init(void){
  int r=0;
  printk(KERN_ALERT "init\n");
  r = alloc_chrdev_region(&dev, FIRST_MINOR, DEV_COUNT, MODULE_NAME);
  if(r<0){
    printk(KERN_WARNING "Cannot allocate char device major");
	  return r;
  }
  scull_major = MAJOR(dev);
  printk(KERN_ALERT "Got major: %d, minor: %d\n", MAJOR(dev), MINOR(dev));
  return 0;

}

static void __exit cleanup(void){
    printk(KERN_ALERT "exit\n");
    unregister_chrdev_region(dev, DEV_COUNT);
}


module_init(init);
module_exit(cleanup);
