#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <linux/cdev.h>

#define MODULE_NAME "scull"
#define DEV_COUNT 4
#define FIRST_MINOR 0


MODULE_LICENSE("Dual BSD/GPL");

int scull_major = 0;
int scull_minor = FIRST_MINOR;
int scull_quantums=1000;
int scull_qsets=100;
struct scull_dev *scull_devs;

struct scull_qset {
  void **data;
  struct scull_qset *next;
};


struct scull_dev {
  struct scull_qset *data;
  int quantum;
  int qset;
  unsigned long size;
  unsigned int access_key;
  struct mutex mutex;
  struct cdev cdev;
};

dev_t dev;


int scull_trim(struct scull_dev* dev){
  int i;
  int qset = dev->qset;
  struct scull_qset *dptr, *next;
  for(dptr=dev->data;dptr;dptr=next){
    if(dptr->data){
      for(i=0;i<qset;i++){
        kfree(dptr->data[i]);
      }
      kfree(dptr->data);
      dptr->data=0;
      next=dptr->next;
      kfree(dptr);
    }
  }
  dev->size=0;
  dev->quantum=scull_quantums;
  dev->qset=scull_qsets;
  dev->data=NULL;
  return 0;
}

int scull_open(struct inode *inode, struct file *filp){
  struct scull_dev *dev;
  dev=container_of(inode->i_cdev, struct scull_dev, cdev);
  filp->private_data=dev;
  if((filp->f_flags & O_ACCMODE) == O_WRONLY){
    scull_trim(dev);
  }
  return 0;
}

int scull_release(struct inode *inode, struct file *filp){

  return 0;
}

struct scull_qset* scull_follow(struct scull_dev* dev, int item){

  struct scull_qset *qset = dev->data;
  if(qset==NULL){
    qset=kmalloc(sizeof(struct scull_qset), GFP_KERNEL);
    if(qset==NULL)return NULL;
    memset(qset, 0, sizeof(struct scull_qset));
  }
  while(item--){
    if(qset->next==NULL){
      qset->next=kmalloc(sizeof(struct scull_qset), GFP_KERNEL);
      if(qset->next==NULL)return NULL;
      memset(qset->next, 0, sizeof(struct scull_qset));
    }
    qset=qset->next;
    continue;
  }
  return qset;
}

ssize_t scull_write(struct file *filp, const char __user *buff, size_t count, loff_t *f_pos){
  struct scull_dev *dev = filp->private_data;
  struct scull_qset *dptr;
  int quantum=dev->quantum, qset=dev->qset;
  int itemsize = quantum*qset;
  int item, rest, s_pos, q_pos;
  ssize_t retval=-ENOMEM;
  if (mutex_lock_interruptible(&dev->mutex)){
    return -ERESTARTSYS;
  }
  if(*f_pos>=dev->size){
    goto out;
  }
  if(*f_pos+count>dev->size){
    count=dev->size-*f_pos;
  }
  item=*f_pos/itemsize;
  rest=*f_pos%itemsize;
  s_pos=rest/quantum;
  q_pos=rest%quantum;

  dptr=scull_follow(dev, item);
  if(dptr==NULL){
    goto out;
  }
  if(!dptr->data){
    dptr->data=kmalloc(qset*sizeof(char*), GFP_KERNEL);
    if(!dptr->data) goto out;
    memset(dptr->data, 0, qset*sizeof(char*));
  }
  if(!dptr->data[s_pos]){
    dptr->data[s_pos]=kmalloc(quantum, GFP_KERNEL);
    if(!dptr->data[s_pos]) goto out;
    memset(dptr->data[s_pos], 0, quantum);
  }
  if(count>quantum-q_pos){
    count=quantum-q_pos;
  }

  if(copy_from_user(dptr->data[s_pos]+q_pos, buff, count)){
    retval = -EFAULT;
  }
  *f_pos+=count;
  if(dev->size < *f_pos){
    dev->size=*f_pos;
  }
  retval=count;
  out:
  mutex_unlock(&dev->mutex);
  return retval;
}

ssize_t scull_read(struct file *filp, char __user *buff, size_t count, loff_t *f_pos){
  struct scull_dev *dev = filp->private_data;
  struct scull_qset *dptr;
  int quantum=dev->quantum, qset=dev->qset;
  int itemsize = quantum*qset;
  int item, rest, s_pos, q_pos;
  ssize_t retval=0;

  if (mutex_lock_interruptible(&dev->mutex)){
    return -ERESTARTSYS;
  }
  if(*f_pos>=dev->size){
    goto out;
  }
  if(*f_pos+count>dev->size){
    count=dev->size-*f_pos;
  }
  item=*f_pos/itemsize;
  rest=*f_pos%itemsize;
  s_pos=rest/quantum;
  q_pos=rest%quantum;

  dptr=scull_follow(dev, item);
  if(dptr==NULL || !dptr->data || !dptr->data[s_pos]){
    goto out;
  }

  if(count>quantum-q_pos){
    count=quantum-q_pos;
  }

  if(copy_to_user(buff, dptr->data[s_pos]+q_pos, count)){
    retval = -EFAULT;
  }
  *f_pos+=count;
  retval=count;
  out:
  mutex_unlock(&dev->mutex);
  return retval;

}


struct file_operations fops = {
  .owner = THIS_MODULE,
  .open = scull_open,
  .release = scull_release,
  .read = scull_read,
  .write = scull_write,
//  .llseek = NULL,//scull_llseek
//  .ioctl = NULL,//scull_ioctl,
};

static void scull_setup_cdev(struct scull_dev *dev, int index){
  int err=0;
  int devno = MKDEV(scull_major, scull_minor+index);
  cdev_init(&dev->cdev, &fops);
  dev->cdev.owner=THIS_MODULE;
  dev->cdev.ops=&fops;
  err=cdev_add(&dev->cdev, devno, index);
  if(err){
    printk(KERN_NOTICE "Err %d!Cannot add scull%d", err, index);
  }
}


static int __init init(void){
  int r=0;
  int i;
  printk(KERN_ALERT "init\n");
  r = alloc_chrdev_region(&dev, FIRST_MINOR, DEV_COUNT, MODULE_NAME);
  if(r<0){
    printk(KERN_WARNING "Cannot allocate char device major");
	  return r;
  }
  scull_major = MAJOR(dev);
  printk(KERN_ALERT "Got major: %d, minor: %d\n", MAJOR(dev), MINOR(dev));
  scull_devs = kmalloc(DEV_COUNT*sizeof(struct scull_dev), GFP_KERNEL);
  if(!scull_devs){
    return -ENOMEM;
  }
  memset(scull_devs, 0, DEV_COUNT*sizeof(struct scull_dev));
  for(i=0;i<DEV_COUNT;i++){
    scull_devs[i].quantum=scull_quantums;
    scull_devs[i].qset=scull_qsets;
    mutex_init(&scull_devs[i].mutex);
    scull_setup_cdev(&scull_devs[i], i);
  }


  return 0;

}

static void __exit cleanup(void){
    printk(KERN_ALERT "exit\n");
    unregister_chrdev_region(dev, DEV_COUNT);
}


module_init(init);
module_exit(cleanup);
