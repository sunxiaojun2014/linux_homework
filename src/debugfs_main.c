/*
 Example of a minimal debugfs
*/
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/debugfs.h>
#include <asm/uaccess.h>

static struct dentry *df_dir = NULL;
int lll_profile;

struct lll_profile_struct {
	u64 age;
};

// write func
static int lll_age_set(void *data, u64 val)
{
	struct lll_profile_struct *lp = (struct lll_profile_struct *)data; 
	lp->age = val;

	return 0;
}

// read func
static int lll_age_get(void *data, u64 *val)
{
	struct lll_profile_struct *lp = (struct lll_profile_struct *)data;
	*val = lp->age;

	return 0;
}

// define file operations 
// macro from linux/fs.h
DEFINE_SIMPLE_ATTRIBUTE(df_age_fops, lll_age_get, lll_age_set, "%llu\n");

static int __init llaolao_init(void)
{
    int n = 0x1937;
    printk(KERN_INFO "Hi, I am llaolao at address 0x%p stack 0x%p.\n",
	    llaolao_init, &n);
    printk(KERN_EMERG "Testing emergent message\n");

	// create /proc/llaolao
	df_dir = debugfs_create_dir("llaolao", 0);
	if(!df_dir)
	{
		printk(KERN_ERR"create dir under debugfs failed\n");
		return -1;
	}
	
	debugfs_create_file("age", 0222, df_dir, &lll_profile, &df_age_fops);
    return 0;
}

static void __exit llaolao_exit(void)
{
    printk("Exiting from 0x%p... Bye, GEDU friends\n", llaolao_exit);
	if(df_dir)
	{
		// clean up all debugfs entries
		debugfs_remove_recursive(df_dir);
	}
}

module_init(llaolao_init);
module_exit(llaolao_exit);

MODULE_AUTHOR("GEDU lab");
MODULE_DESCRIPTION("LKM example - llaolao");
MODULE_LICENSE("GPL");
