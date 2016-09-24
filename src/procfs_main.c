/*
 Example of a minimal character device driver 
*/
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>

static struct proc_dir_entry *proc_lll_entry = NULL;

static ssize_t proc_lll_read(struct file *filp, char __user *buf, size_t count, loff_t *offp)
{
	int n = 0, ret;
	char secrets[100];

	printk(KERN_INFO "proc_lll_read is called file %p, buf %p, count %d, off %llx\n", filp, buf, count, *offp);
	sprintf(secrets, "kernel secrets balabala %s...\n", filp->f_path.dentry->d_iname);
	
	n = strlen(secrets);
	if(*offp < n)
	{
		copy_to_user(buf, secrets, n + 1);
		*offp = ret = n + 1;
	} 
	else 
	{
		ret = 0;
	}

	return ret;
}

static const struct file_operations proc_lll_fops = {
	.owner = THIS_MODULE,
	.read  = proc_lll_read,
};

static int __init llaolao_init(void)
{
    int n = 0x1937;
    printk(KERN_INFO "Hi, I am llaolao at address 0x%p stack 0x%p.\n",
	    llaolao_init, &n);
    printk(KERN_EMERG "Testing emergent message\n");

	// create /proc/llaolao
	proc_lll_entry = proc_create("llaolao", 0, NULL, &proc_lll_fops);
    return 0;
}

static void __exit llaolao_exit(void)
{
    printk("Exiting from 0x%p... Bye, GEDU friends\n", llaolao_exit);
	if(proc_lll_entry)
	{
		proc_remove(proc_lll_entry);
	}
}

module_init(llaolao_init);
module_exit(llaolao_exit);

MODULE_AUTHOR("GEDU lab");
MODULE_DESCRIPTION("LKM example - llaolao");
MODULE_LICENSE("GPL");
