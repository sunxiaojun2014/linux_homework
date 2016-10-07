/*
 Example of a character device
*/
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <asm/uaccess.h>

#define NUM_DEVICES 		6 
#define DEV_DATA_LENGTH 	128	
#define DEVICE_NAME 		"huadeng"
#define MAJOR_NUM 			88

// ioctl macro
#define HUADENG_LUMINANCE 0
#define HUADENG_IOC_MAGIC 'o'

/*
* S means "Set" through a pointer
* T means "Tell" directly with the argument value
* G means "Get" reply by setting through a pointer
* Q means "Query" response is on the return value
* X means "eXchage" switch G and S atomically
* H means "sHift" switch T and Q atomically
*/
#define HUADENG_IOCRESET			_IO(HUADENG_IOC_MAGIC, 0)
#define HUADENG_IOCSLUMINANCE 		_IOW(HUADENG_IOC_MAGIC, 1, int)
#define HUADENG_IOCTLUMINANCE		_IO(HUADENG_IOC_MAGIC, 2)
#define HUADENG_IOCGLUMINANCE		_IOR(HUADENG_IOC_MAGIC, 3, int)
#define HUADENG_IOCQLUMINANCE		_IO(HUADENG_IOC_MAGIC, 4)
#define HUADENG_IOCXLUMINANCE		_IOWR(HUADENG_IOC_MAGIC, 5, int)
#define HUADENG_IOCHLUMINANCE		_IO(HUADENG_IOC_MAGIC, 6)

// per huadeng device struct
struct huadeng_dev {
	unsigned short current_pointer;
	unsigned int size;
	int number;
	unsigned int luminance;
	struct cdev cdev;
	char name[10];
	char data[DEV_DATA_LENGTH];
	/* ... */  /* mutexes, spinlocks, wait queues ... */

} *hd_devp[NUM_DEVICES];

static struct class *huadeng_class = NULL;

static int 
huadeng_open(struct inode *inode, struct file *file)
{
	struct huadeng_dev *hd_devp;
	pr_info("%s\n", __func__);

	/* GEt the per-device structure that contains this device*/
	hd_devp = container_of(inode->i_cdev, struct huadeng_dev, cdev);

	/* Easy access to hd_devp from rest of the entry points*/
	file->private_data = hd_devp;

	/* Initialize some fields */
	hd_devp->size = DEV_DATA_LENGTH;
	hd_devp->current_pointer = 0;

	return 0;
}

static ssize_t 
huadeng_read(struct file *file, char __user *buffer, size_t count, loff_t *offset)
{
	struct huadeng_dev *hd_devp = file->private_data;
	pr_info("%s count %u, +%llu\n", __func__, count, *offset);

	if (*offset >= hd_devp->size) 
	{
		return 0; /* EOF */
	}

	/* Adjust count if its edges past the end of the data region*/
	if (*offset + count > hd_devp->size)
	{
		count = hd_devp->size - *offset;
	}

	/* Copy the read bytes to the user buffer */
	if (copy_to_user(buffer, (void*)(hd_devp->data + *offset), count) != 0)
	{
		return -EIO;
	}

	*offset += count;

	return count;
}

static ssize_t 
huadeng_write(struct file *file, const char *buffer, size_t length, loff_t *offset )
{
	struct huadeng_dev *hd_devp = file->private_data;
	pr_info("%s %u +%llx\n", __func__, length, *offset);
	if (*offset >= hd_devp->size)
	{
		return 0;
	}
	
	if (*offset + length > hd_devp->size)
	{
		length = hd_devp->size - *offset;
	}

	if (copy_from_user(hd_devp->data + *offset, buffer, length) != 0)
	{
		printk(KERN_ERR"copy_from_user faile\n");
		return -EFAULT;
	}

	return length;
}

static int 
huadeng_release(struct inode *inode, struct file *file)
{
	struct huadeng_dev *hd_devp = file->private_data; 

	// reset current pointer
	hd_devp->current_pointer = 0;	

	return 0;
}

static long
huadeng_ioctl(/*struct inode *inode,*/ struct file *file, 
	unsigned int cmd, unsigned long arg)
{
	long retval = 0;
	int tmp = 0;
	struct huadeng_dev *hd_devp = file->private_data;
	pr_info("%s cmd 0x%x arg %lx, luminance %d\n", 
		__func__, cmd, arg, hd_devp->luminance);

	switch(cmd)
	{
		case HUADENG_IOCRESET:
			hd_devp->luminance = HUADENG_LUMINANCE;
			break;
		case HUADENG_IOCSLUMINANCE: /* Set: arg points to the value*/
			if (!capable(CAP_SYS_ADMIN))
				return -EPERM;
			retval = __get_user(hd_devp->luminance, (int __user*)arg);
			break;
		case HUADENG_IOCTLUMINANCE: /* Tell: arg is the value*/
			if (!capable(CAP_SYS_ADMIN))
				return -EPERM;
			hd_devp->luminance = arg;
			break;
		case HUADENG_IOCGLUMINANCE: /* Get: arg is the pointer to result*/
			retval = __put_user(hd_devp->luminance, (int __user*)arg);
			break;
		case HUADENG_IOCQLUMINANCE: /* Query: return it (it's positive)*/
			return hd_devp->luminance;
		case HUADENG_IOCXLUMINANCE: /* eXchange: use arg as pointer*/
			if (!capable(CAP_SYS_ADMIN))
				return -EPERM;
			tmp = hd_devp->luminance;
			retval = __get_user(hd_devp->luminance, (int __user*)arg);
			if (retval == 0)
				retval = __put_user(tmp, (int __user*)arg);
			break;
		case HUADENG_IOCHLUMINANCE: /* sHift: like Tell + Query*/
			if (!capable(CAP_SYS_ADMIN))
				return -EPERM;
			tmp = hd_devp->luminance;
			hd_devp->luminance = arg;
			return tmp;
		default: /* redundant, as cmd was checked against MAXNR*/
			return -ENOTTY;
	}

	return retval;
}

const static struct file_operations huadeng_fops = {
	.owner 	 = THIS_MODULE,
	.open  	 = huadeng_open,
	.release = huadeng_release,
	.read    = huadeng_read,
	.write   = huadeng_write,
	.unlocked_ioctl = huadeng_ioctl,
}; 

// static dev_t hd_dev_number;
static int __init llaolao_init(void)
{
	int i;
	/*
	if (alloc_chrdev_region(&hd_dev_number, 0, NUM_DEVICES, DEVICE_NAME) < 0) {
	printk(KERN_ERR"alloc device major number failed\n");
	return -1;
}*/
	// cerate huadeng device class & register class 
	huadeng_class = class_create(THIS_MODULE, DEVICE_NAME);
	if(!huadeng_class)
	{
		printk(KERN_ERR"create huadeng class failed\n");
		return -1;
	}

	// allocate memory for every huadeng device structure
	for(i = 0; i < NUM_DEVICES; i++)
	{
		hd_devp[i] = kmalloc(sizeof(struct huadeng_dev), GFP_KERNEL);
		if(!hd_devp[i])
		{
			printk(KERN_ERR"allocate huadeng dev struct failed\n");
			return -ENOMEM;
		}

		// connect the file operations with the cdev
		cdev_init(&hd_devp[i]->cdev, &huadeng_fops);
		hd_devp[i]->number = i;
		hd_devp[i]->cdev.owner = THIS_MODULE;
		
		/* Connect the major/minor number to the cdev */
		cdev_add(&hd_devp[i]->cdev, MKDEV(MAJOR_NUM, i), 1);
		// cdev_add(&hd_devp[i]->cdev, (hd_dev_number + i), 1);

		// Send uevents to udev, so it'll create /dev nodes
		device_create(huadeng_class, NULL, MKDEV(MAJOR_NUM, i), NULL, "huadeng%d", i);
		// device_create(huadeng_class, NULL, MKDEV(hd_dev_number, i), NULL, "huadeng%d", i);
	}
	
	printk(KERN_INFO"huadeng device driver init succ\n");

    return 0;
}

static void __exit llaolao_exit(void)
{
	int i;

	printk("huadeng device unloading...\n");

	unregister_chrdev_region(MAJOR_NUM, NUM_DEVICES);
	// unregister_chrdev_region(hd_dev_number, NUM_DEVICES);

	for(i = 0; i < NUM_DEVICES; i++)
	{
		device_destroy(huadeng_class, MKDEV(MAJOR_NUM, i));
		// device_destroy(huadeng_class, MKDEV(hd_dev_number, i));
		cdev_del(&hd_devp[i]->cdev);
		kfree(hd_devp[i]);
	}

	// destroy huadeng class 
	class_destroy(huadeng_class);
	printk("huadeng device unload succ\n");

	return ;
}

module_init(llaolao_init);
module_exit(llaolao_exit);

MODULE_AUTHOR("GEDU lab");
MODULE_DESCRIPTION("LKM example - llaolao");
MODULE_LICENSE("GPL");
