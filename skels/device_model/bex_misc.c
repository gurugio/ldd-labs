#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>
#include "bex.h"

MODULE_DESCRIPTION("BEX misc driver");
MODULE_AUTHOR("Kernel Hacker");
MODULE_LICENSE("GPL");

#define BUF_SIZE 1024

struct bex_misc_device {
	struct miscdevice misc;
	struct bex_device *dev;
	char buf[BUF_SIZE];
};

#define to_bex_misc_dev(dev) container_of(dev, struct bex_misc_device, dev)

static int my_open(struct inode *inode, struct file *file)
{
	return 0;
}

static int my_release(struct inode *inode, struct file *file)
{
	return 0;
}

static int my_read(struct file *file, char __user *user_buffer,
		   size_t size, loff_t *offset)
{
	struct bex_misc_device *bmd = (struct bex_misc_device *)file->private_data;
	ssize_t len = min(sizeof(bmd->buf) - (ssize_t)*offset, size);

	if (len <= 0)
		return 0;

	if (copy_to_user(user_buffer, bmd->buf + *offset, len))
		return -EFAULT;

	*offset += len;
	return len;
}

static int my_write(struct file *file, const char __user *user_buffer,
		    size_t size, loff_t *offset)
{
	struct bex_misc_device *bmd = (struct bex_misc_device *)file->private_data;
	ssize_t len = min(sizeof(bmd->buf) - (ssize_t)*offset, size);

	if (len <= 0)
		return 0;

	if (copy_from_user(bmd->buf + *offset, user_buffer, len))
		return -EFAULT;

	*offset += len;
	return len;
}

struct file_operations bex_misc_fops = {
	.owner = THIS_MODULE,
	.open = my_open,
	.read = my_read,
	.write = my_write,
	.release = my_release,
};

static int bex_misc_count;

int bex_misc_probe(struct bex_device *dev)
{
	struct bex_misc_device *bmd;
	char buf[32];
	int ret;

	dev_info(&dev->dev, "%s: %s %d\n", __func__, dev->type, dev->version);

	/* TODO 6: refuse the probe is version > 1 */

	bmd = kzalloc(sizeof(struct miscdevice), GFP_KERNEL);
	if (!bmd)
		return -ENOMEM;

	bmd->misc.minor = MISC_DYNAMIC_MINOR;
	snprintf(buf, sizeof(buf), "bex-misc-%d", bex_misc_count++);
	bmd->misc.name = kstrdup(buf, GFP_KERNEL);
	bmd->misc.parent = &dev->dev;
	bmd->misc.fops = &bex_misc_fops;
	bmd->dev = dev;
	dev_set_drvdata(&dev->dev, bmd);

	/* TODO 6: register the misc device */

	return 0;
}

void bex_misc_remove(struct bex_device *dev)
{
	struct bex_misc_device *bmd;

	bmd = (struct bex_misc_device *)dev_get_drvdata(&dev->dev);

	/* TODO 6: deregister the misc device */
	kfree(bmd);
}

struct bex_driver bex_misc_driver = {
	.type = "misc",
	.probe = bex_misc_probe,
	.remove = bex_misc_remove,
	.driver = {
		.owner = THIS_MODULE,
		.name = "bex_misc",
	},
};

static int my_init(void)
{
	int err;

	/* TODO 4: register the driver */

	return 0;
}

static void my_exit(void)
{
	/* TODO 4: unregister the driver */
}

module_init(my_init);
module_exit(my_exit);
