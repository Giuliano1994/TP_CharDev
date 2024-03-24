#include <linux/atomic.h>
#include <linux/cdev.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/types.h>
#include <linux/uaccess.h>
#include <asm/errno.h>

#define DEVICE_NAME "chardev"
#define BUF_LEN 80

//static int Device_Open = 0;
//static char Message[BUF_LEN + 1];
//static int Message_Size = 0;
static struct class *cls;
static int Major;


static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char __user *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char __user *, size_t, loff_t *);


enum {
CDEV_NOT_USED = 0,
CDEV_EXCLUSIVE_OPEN = 1,
};

//static atomic_t already_open = ATOMIC_INIT(CDEV_NOT_USED);







static int device_open(struct inode *inode, struct file *file)
{

/*
    if (Device_Open)
        return -EBUSY;

    Device_Open++;
    */
    return 0;
    
}

static int device_release(struct inode *inode, struct file *file)
{
    /*
    Device_Open--;
     */
    return 0;   
   
}

static ssize_t device_read(struct file *filp, char *buffer, size_t length, loff_t *offset)
{
    /*
    int bytes_read = 0;

    if (*offset >= Message_Size)
        return 0;

    while (length && (*offset < Message_Size)) {
        put_user(Message[*offset], buffer++);
        (*offset)++;
        length--;
        bytes_read++;
    }

    return bytes_read;
    */
   return 0;
}

static ssize_t device_write(struct file *filp, const char *buff, size_t len, loff_t *off)
{
    /*
    int i;
    for (i = 0; i < len && i < BUF_LEN; ++i) {
        get_user(Message[i], buff + i);
    }
    Message_Size = i;
    return i;
    */
   return 0;
}

static struct file_operations fops = {
    .read = device_read,
    .write = device_write,
    .open = device_open,
    .release = device_release
};


/* Registro un driver con las operaciones a realizar si es menos que 0 tiro error sino el print*/
static int __init chardev_init(void)
{ 
    Major = register_chrdev(0, DEVICE_NAME, &fops);
    if (Major < 0) {
        pr_alert("Fallo el registro de Char Device %d\n", Major);
        return Major;
    }
    
    pr_info("Se asigna el numero a Major %d.\n", Major);


    cls = class_create(THIS_MODULE, DEVICE_NAME);
    device_create(cls, NULL, MKDEV(Major, 0), NULL, DEVICE_NAME);
    


    pr_info("Device created on /dev/%s\n", DEVICE_NAME);

    return 0;
}

/* Destruyo el driver y luego lo desregistro*/
static void __exit chardev_exit(void)
{
    device_destroy(cls, MKDEV(Major, 0));
    class_destroy(cls);
    
    unregister_chrdev(Major, DEVICE_NAME);

}

module_init(chardev_init);
module_exit(chardev_exit);
