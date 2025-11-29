/**
 * @file aesdchar.c
 * @brief Functions and data related to the AESD char driver implementation
 *
 * Based on the implementation of the "scull" device driver, found in
 * Linux Device Drivers example code.
 *
 * @author Dan Walkes
 * @date 2019-10-22
 * @copyright Copyright (c) 2019
 *
 */
#include <linux/version.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/printk.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/mutex.h>
#include <linux/string.h>
#include <linux/fs.h> // file_operations
#include "aesd_ioctl.h"
#include "aesdchar.h"
#include "aesd-circular-buffer.h"
int aesd_major =   0; // use dynamic major
int aesd_minor =   0;

#define NUM_DEV 1 //number of devices to add

MODULE_AUTHOR("Lucas Rettore"); 
MODULE_LICENSE("Dual BSD/GPL");

static struct aesd_dev aesd_device;
static struct class *dev_class;



int aesd_open(struct inode *inode, struct file *filp)
{
    PDEBUG("open");
    
    int minor = iminor(inode);
     if (minor >= NUM_DEV)
        return -ENODEV;

    struct aesd_dev *dev; /* device information */

     
    dev = container_of(inode->i_cdev, struct aesd_dev, cdev);
    filp->private_data = dev; /* for other methods */

   
    return 0; /* success */ 


}

int aesd_release(struct inode *inode, struct file *filp)
{
    PDEBUG("release");
  
    return 0;
}

loff_t aesd_llseek(struct file *filp, loff_t f_pos, int whence)
{
    struct aesd_dev *dev; 
    dev = filp->private_data; 

    // Lock
    mutex_lock(&dev->lock);


    loff_t ret =  fixed_size_llseek(filp, f_pos, whence, dev->size);

    mutex_unlock(&dev->lock);
    return ret;
}

ssize_t aesd_read(struct file *filp, char __user *buf, size_t count,
                loff_t *f_pos)
{
    PDEBUG("read %zu bytes with offset %lld",count,*f_pos);
    
   // Make vars
    ssize_t retval = 0;
    ssize_t offset_rtn = 0;
    struct aesd_buffer_entry *entry;

    size_t bytes_remaining = count;
    size_t bytes_to_send;
    size_t bytes_available = 0;
   

    struct aesd_dev *dev; 
    dev = filp->private_data; 

      // Lock
    mutex_lock(&dev->lock);



    while (bytes_remaining > 0 )
    {
       
        
        entry =  aesd_circular_buffer_find_entry_offset_for_fpos(&dev->cbuf,
                                                        *f_pos,
                                                        &offset_rtn);
        if (entry ==NULL)
            break;

        bytes_available = entry->size - offset_rtn;//compute bytes from current entry  avaible to send

        if (bytes_remaining> bytes_available)
        {
            bytes_to_send = bytes_available;
 
        }
        else
        {
            bytes_to_send = bytes_remaining;
        }

        //compute remaining butes to send
        bytes_remaining = bytes_remaining - bytes_to_send;

        //copy to user space
        if(copy_to_user(buf, entry->buffptr+(offset_rtn), bytes_to_send)!=0)
        {
            printk(KERN_ERR "Failed to copy data to user space\n");
            retval = -EFAULT;
            goto unlock;
        }
           
        //advance file position
        *f_pos += bytes_to_send;
        //advance buffer to not overwrite data
        buf += bytes_to_send;
       
        retval = retval+bytes_to_send;
    }
   

    unlock:
    mutex_unlock(&dev->lock);
    return retval;
}

ssize_t aesd_write(struct file *filp, const char __user *buf, size_t count,
                loff_t *f_pos)
{
    struct aesd_dev *dev; /* device information */
    dev = filp->private_data; 


    
   
   mutex_lock(&dev->lock);
    ssize_t retval = -ENOMEM;

    char *kbuf = kmalloc(count, GFP_KERNEL);
    if (!kbuf)
    {
        mutex_unlock(&dev->lock);
        return -ENOMEM;
    }
        

    if (copy_from_user(kbuf, buf, count)!=0) {

        kfree(kbuf);
        mutex_unlock(&dev->lock);
        return -EFAULT;
    }


    
    //allocate memory to temporary buffer to expand tmp_entry
    char *tempBuf = krealloc(dev->tmp_entry.buffptr, count+dev->tmp_entry.size, GFP_KERNEL);
    if (!tempBuf)
    {
        kfree(kbuf);
        mutex_unlock(&dev->lock);
        return -ENOMEM;
    }
    
    //tmp_entry points to the new allocated expanded memory
    dev->tmp_entry.buffptr = tempBuf;
    

    //to append data from user to the end of temp buffer needs offset --> dev->tmp_entry.size
    memcpy(dev->tmp_entry.buffptr + dev->tmp_entry.size, kbuf, count);

    //update size
    dev->tmp_entry.size += count;
   
    char endStr = '\n';
   

    //Check if the string is empty to avoid accessing an invalid index
    if (count > 0) {
        printk(KERN_DEBUG "aesd_write: last char of kbuf = '%c' (0x%x)\n", kbuf[count-1], kbuf[count-1]);
        // Access the last character and compare
        if (kbuf[count-1] == endStr) {

            dev->entry.buffptr= dev->tmp_entry.buffptr;
            dev->entry.size=dev->tmp_entry.size;
            printk(KERN_DEBUG "aesd_write: entry_buf contents: %.*s\n", (int)dev->tmp_entry.size, dev->tmp_entry.buffptr);
            printk(KERN_DEBUG "aesd_write: countOld = %zu\n", dev->tmp_entry.size);

            aesd_circular_buffer_add_entry(&dev->cbuf,&dev->entry);
            //kfree(dev->tmp_entry.buffptr);
            dev->size += dev->entry.size;
            dev->tmp_entry.buffptr = NULL;
            dev->tmp_entry.size = 0;
             
        } 
    }

  
    PDEBUG("write %zu bytes with offset %lld",count,*f_pos);
    mutex_unlock(&dev->lock);

    kfree(kbuf);
    retval=count;

    //to do yet
    return retval;
}

int aesd_adjust_file_offset(struct file *filp, uint32_t write_cmd, uint32_t write_cmd_offset)
{
    struct aesd_dev *dev; /* device information */
    dev = filp->private_data; 


    if (mutex_lock_interruptible(&dev->lock)) 
    {
        return -ERESTARTSYS;
    }

    if (write_cmd >= AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED)
    {
        mutex_unlock(&dev->lock);
        return -1;
    }
   
    
    if (write_cmd_offset >= dev->cbuf.entry[write_cmd].size)
    {
        mutex_unlock(&dev->lock);
        return -1;
    }

    size_t i = 0;
    size_t entry_size_total = 0;

    while (i< AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED)
    {

        entry_size_total += dev->cbuf.entry[i].size;
        if (i == write_cmd)
            break; 
        i++;
    }

    filp->f_pos = entry_size_total + write_cmd_offset;

    mutex_unlock(&dev->lock);

    return 0;

}

long aesd_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    struct aesd_dev *dev; /* device information */
    dev = filp->private_data; 
    long ret = -ENOTTY;

    switch (cmd) {
    case AESDCHAR_IOCSEEKTO:
    struct aesd_seekto seekto; 
        if (copy_from_user(&seekto, (const void __user *)arg, sizeof(seekto))!=0)
        {
            return -EFAULT;
        }
        else
        {
            ret = aesd_adjust_file_offset(filp, seekto.write_cmd, seekto.write_cmd_offset);
            printk(KERN_INFO "Value from user: %d\n", seekto.write_cmd);
        }
       
        break;
    }


    return ret;

}


struct file_operations aesd_fops = {
    .owner =    THIS_MODULE,
    .read =     aesd_read,
    .write =    aesd_write,
    .open =     aesd_open,
    .release =  aesd_release,
    .llseek =   aesd_llseek,
    .unlocked_ioctl = aesd_ioctl
};

static int aesd_setup_cdev(struct aesd_dev *dev)
{
    int err, devno = MKDEV(aesd_major, aesd_minor);

    cdev_init(&dev->cdev, &aesd_fops);
    dev->cdev.owner = THIS_MODULE;
    dev->cdev.ops = &aesd_fops;
    err = cdev_add (&dev->cdev, devno, 1);
    if (err) {
        printk(KERN_ERR "Error %d adding aesd cdev", err);
    }

    /* Creating struct class */
    #if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 4, 0)
        // For Kernel 6.4.0 and newer (uses only one argument)
        dev_class = class_create("aesd_class");
    #else
        // For older kernels (uses two arguments)
        dev_class = class_create(THIS_MODULE, "aesd_class");
    #endif

    //Creating device - not using mknod
    if ((device_create(dev_class, NULL, devno, NULL, "aesdchar")) == NULL) {
        printk(KERN_INFO "Cannot create the Device\n");
        goto r_device;
    }

    printk(KERN_INFO "Device Driver Insert Done\n");
    return 0;

    r_device:
        class_destroy(dev_class);
    r_class:
        cdev_del(&dev->cdev);
        err = -1;


    return err;
}



int aesd_init_module(void)
{
    dev_t dev = 0;
    int result;
    result = alloc_chrdev_region(&dev, aesd_minor, NUM_DEV,
            "aesdchar");
    aesd_major = MAJOR(dev);
    if (result < 0) {
        printk(KERN_WARNING "Can't get major %d\n", aesd_major);
        return result;
    }

 
    memset(&aesd_device,0,sizeof(struct aesd_dev));

    // AESD-specific initialization
    aesd_circular_buffer_init(&aesd_device.cbuf);
    mutex_init(&aesd_device.lock);
    aesd_device.tmp_entry.buffptr = NULL;
    aesd_device.tmp_entry.size = 0;
    aesd_device.size = 0;

    result = aesd_setup_cdev(&aesd_device);

    if( result ) {
        unregister_chrdev_region(dev, 1);
    }
    return result;

}

void aesd_cleanup_module(void)
{
    dev_t devno = MKDEV(aesd_major, aesd_minor);
    device_destroy(dev_class, devno);
    class_destroy(dev_class);
    cdev_del(&aesd_device.cdev);
    uint8_t index;
    struct aesd_buffer_entry *entry;
     AESD_CIRCULAR_BUFFER_FOREACH(entry, &aesd_device.cbuf, index) 
    {
        kfree(entry->buffptr);
    }
    mutex_destroy(&aesd_device.lock);

    unregister_chrdev_region(devno, 1);
}



module_init(aesd_init_module);
module_exit(aesd_cleanup_module);
