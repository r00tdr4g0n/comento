#include <linux/module.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/spinlock.h>
#include <linux/hashtable.h>

#define KEYRINGCTL_DEVICE_NAME		"keyringctl"
#define KEYRINGCTL_CLASS_NAME		"keyringctl"
#define KEYRING_DEVICE_NAME		"keyring"
#define KEYRING_CLASS_NAME		"keyring"
#define KEYRING_BUF_SIZE		32
#define KEYRINGCTL_MAGIC_NUMBER		'K'
#define KEYRINGCTL_DEVICE_IOCTL_ADD	_IOW(KEYRINGCTL_MAGIC_NUMBER, 0, unsigned int)
#define KEYRINGCTL_DEVICE_IOCTL_DEL	_IOW(KEYRINGCTL_MAGIC_NUMBER, 1, unsigned int)
#define KEYRINGCTL_DEVICE_IOCTL_SHOW	_IO(KEYRINGCTL_MAGIC_NUMBER, 2)
#define KEYRING_HASH_SIZE		10

static DEFINE_HASHTABLE(keyring_table, KEYRING_HASH_SIZE);
static DEFINE_RWLOCK(keyring_device_rwlock);
static int 		keyringctl_device_major;
static int 		keyring_device_major;
static int		keyring_device_minor;
static struct class 	*keyringctl_class;
static struct device 	*keyringctl_device;
static struct class	*keyring_class;
static struct device 	*keyring_device;

/*
 * hashtable
 */
struct keyring_item {
	int key;
	char *data;
	struct hlist_node hash;
};

void add_item(int key, char* data)
{
	struct keyring_item *new_item;
	new_item = kmalloc(sizeof(struct keyring_item), GFP_KERNEL);
	new_item->key = key;
	new_item->data = data;
	hash_add(keyring_table, &new_item->hash, new_item->key);
	printk(KERN_DEBUG "[%s] Add %s in %d", KEYRING_DEVICE_NAME, new_item->data, new_item->key);
}

char* get_item(int key)
{
	struct keyring_item *item;
	hash_for_each_possible(keyring_table, item, hash, key) {
		return item->data;
	}

	return NULL;
}

/*
 * keyringctl
 */
static int 	keyringctl_device_open(struct inode *inode, struct file *fp);
static int 	keyringctl_device_release(struct inode *inode, struct file *fp);
static ssize_t 	keyringctl_device_read(struct file *fp, char __user *buf, size_t len, loff_t *ppos);
static ssize_t 	keyringctl_device_write(struct file *fp, const char __user *buf, size_t len, loff_t *ppos);
static long 	keyringctl_device_ioctl(struct file *fp, unsigned int cmd, unsigned long arg);

static struct file_operations keyringctl_device_fops = {
	.open 		= keyringctl_device_open,
	.release 	= keyringctl_device_release,
	.read		= keyringctl_device_read,
	.write		= keyringctl_device_write,
	.unlocked_ioctl = keyringctl_device_ioctl,
};

static int keyringctl_device_open(struct inode *inode, struct file *fp)
{
	printk(KERN_DEBUG "[%s] %s", KEYRINGCTL_DEVICE_NAME, __func__);
	return 0;
}

static int keyringctl_device_release(struct inode *inode, struct file *fp)
{
	printk(KERN_DEBUG "[%s] %s", KEYRINGCTL_DEVICE_NAME, __func__);
	return 0;
}

static ssize_t keyringctl_device_read(struct file *fp, char __user *buf, size_t len, loff_t *ppos)
{
	printk(KERN_DEBUG "[%s] %s", KEYRINGCTL_DEVICE_NAME, __func__);
	return -EINVAL;
}

static ssize_t keyringctl_device_write(struct file *fp, const char __user *buf, size_t len, loff_t *ppos)
{
	printk(KERN_DEBUG "[%s] %s", KEYRINGCTL_DEVICE_NAME, __func__);
	return -EINVAL;
}

int create_keyring_device(int num)
{
	keyring_device = device_create(
			keyring_class,
			NULL,
			MKDEV(keyring_device_major, num),
			NULL,
			"%s%d", KEYRING_DEVICE_NAME, num);

	if (IS_ERR(keyring_device)) {
		unregister_chrdev(keyring_device_major, KEYRING_DEVICE_NAME);
		class_destroy(keyring_class);
		return PTR_ERR(keyring_device);
	}

	return 0;
}

void delete_keyring_device(int num)
{
	struct keyring_item *del_item = 0;

	hash_for_each_possible(keyring_table, del_item, hash, num) {
		kfree(del_item->data);
		hash_del(&del_item->hash);
	}

	device_destroy(keyring_class, MKDEV(keyring_device_major, num));
}

static long keyringctl_device_ioctl(struct file *fp, unsigned int cmd, unsigned long arg)
{
	unsigned int 	num = 0;
	int 		ret = 0;
	char 		*data = 0;
	
	switch (cmd) {
		case KEYRINGCTL_DEVICE_IOCTL_ADD:
			if (copy_from_user(&num, (void __user *)arg, sizeof(unsigned int))) {
				return -EINVAL;
			}

			ret = create_keyring_device(num);	
			
			if (ret) {
				return ret;
			}

			data = kmalloc(KEYRING_BUF_SIZE, GFP_KERNEL);
			memset(data, 0, KEYRING_BUF_SIZE);
			add_item(num, data);

			printk(KERN_DEBUG "[%s] Add keyring%d", KEYRINGCTL_DEVICE_NAME, num);
			
			break;
		case KEYRINGCTL_DEVICE_IOCTL_DEL:
			if (copy_from_user(&num, (void __user *)arg, sizeof(unsigned int))) {
				return -EINVAL;
			}
			
			delete_keyring_device(num);
				
			printk(KERN_DEBUG "[%s] Delete keyring%d", KEYRINGCTL_DEVICE_NAME, num);
			break;
		case KEYRINGCTL_DEVICE_IOCTL_SHOW:
			printk(KERN_DEBUG "[%s] SHOW", KEYRINGCTL_DEVICE_NAME);
			break;
		default:
			return -EINVAL;
	}

	return 0;
}

/*
 * keyring<number>
 */
static int 	keyring_device_open(struct inode *inode, struct file *fp);
static int 	keyring_device_release(struct inode *inode, struct file *fp);
static ssize_t 	keyring_device_read(struct file *fp, char __user *buf, size_t len, loff_t *ppos);
static ssize_t 	keyring_device_write(struct file *fp, const char __user *buf, size_t len, loff_t *ppos);

static struct file_operations keyring_device_fops = {
	.open 		= keyring_device_open,
	.release 	= keyring_device_release,
	.read		= keyring_device_read,
	.write		= keyring_device_write,
};

static int keyring_device_open(struct inode *inode, struct file *fp)
{
	keyring_device_minor = iminor(inode);

	printk(KERN_DEBUG "[%s] %s", KEYRING_DEVICE_NAME, __func__);
	return 0;
}

static int keyring_device_release(struct inode *inode, struct file *fp)
{
	printk(KERN_DEBUG "[%s] %s", KEYRING_DEVICE_NAME, __func__);
	return 0;
}

static ssize_t keyring_device_read(struct file *fp, char __user *buf, size_t len, loff_t *ppos)
{
	int 	written_bytes = 0;
	char*	key_data = 0; 

	printk(KERN_DEBUG "[%s] %s", KEYRING_DEVICE_NAME, __func__);

	read_lock(&keyring_device_rwlock);

	if (KEYRING_BUF_SIZE <= len + *ppos) {
		len = KEYRING_BUF_SIZE - *ppos;
	}

	key_data = get_item(keyring_device_minor);
	
	if (key_data)
		written_bytes = len - copy_to_user(buf, get_item(keyring_device_minor) + *ppos, len);

	*ppos += written_bytes;

	read_unlock(&keyring_device_rwlock);

	return written_bytes;
}

static ssize_t keyring_device_write(struct file *fp, const char __user *buf, size_t len, loff_t *ppos)
{
	int 	read_bytes = 0;
	char* 	key_data = 0;

	printk(KERN_DEBUG "[%s] %s", KEYRING_DEVICE_NAME, __func__);

	write_lock(&keyring_device_rwlock);

	if (KEYRING_BUF_SIZE <= len + *ppos) {
		len = KEYRING_BUF_SIZE - *ppos;
	}
	
	key_data = get_item(keyring_device_minor);
	read_bytes = len - copy_from_user(key_data + *ppos, buf, len);
	*ppos += read_bytes;

	write_unlock(&keyring_device_rwlock);

	return read_bytes;
}


/*
 * initialize module
 */
static int __init keyring_module_init(void)
{
	int ret = 0;
	int minor = 0;

	printk(KERN_DEBUG "[%s] %s", KEYRINGCTL_DEVICE_NAME, __func__);
	
	/*
	 * create class, device for keyringctl
	 */
	keyringctl_device_major = register_chrdev(0, KEYRINGCTL_DEVICE_NAME, &keyringctl_device_fops);

	if (0 <= keyringctl_device_major)  {
		keyringctl_class = class_create(THIS_MODULE, KEYRINGCTL_CLASS_NAME);

		if (!IS_ERR(keyringctl_class)) {
			keyringctl_device = device_create(
					keyringctl_class,
					NULL,
					MKDEV(keyringctl_device_major, minor),
					NULL,
					"%s", KEYRINGCTL_DEVICE_NAME);

			if (IS_ERR(keyringctl_device)) {
				printk(KERN_ERR "[%s] Failed to create device", KEYRINGCTL_DEVICE_NAME);
				class_destroy(keyringctl_class);
				unregister_chrdev(keyringctl_device_major, KEYRINGCTL_DEVICE_NAME);
				
				return PTR_ERR(keyringctl_device);
			}
		}
		else {
			printk(KERN_ERR "[%s] Failed to create class", KEYRINGCTL_DEVICE_NAME);
			
			unregister_chrdev(keyringctl_device_major, KEYRINGCTL_DEVICE_NAME);
			
			return PTR_ERR(keyringctl_class);
		}
	}
	else {
		printk(KERN_ERR "[%s] Failed to get major number", KEYRINGCTL_DEVICE_NAME);
		
		return keyringctl_device_major;
	}


	/*
	 * create class, device for keyring
	 */
	keyring_device_major = register_chrdev(0, KEYRING_DEVICE_NAME, &keyring_device_fops);

	if (0 <= keyring_device_major) {
		keyring_class = class_create(THIS_MODULE, KEYRING_DEVICE_NAME);	

		if (IS_ERR(keyring_class)) {
			printk(KERN_ERR "[%s] Failed to create class", KEYRING_DEVICE_NAME);

			unregister_chrdev(keyring_device_major, KEYRING_DEVICE_NAME);

			device_destroy(keyringctl_class, MKDEV(keyringctl_device_major, minor));
			class_destroy(keyringctl_class);
			unregister_chrdev(keyringctl_device_major, KEYRINGCTL_DEVICE_NAME);

			return PTR_ERR(keyring_class);
		}
	}
	else {
		printk(KERN_ERR "[%s] Failed to get major number", KEYRING_DEVICE_NAME);

		device_destroy(keyringctl_class, MKDEV(keyringctl_device_major, minor));
		class_destroy(keyringctl_class);
		unregister_chrdev(keyringctl_device_major, KEYRINGCTL_DEVICE_NAME);

		return keyring_device_major;
	}

	return ret;
}

/*
 * Exit module
 */
static void __exit keyring_module_exit(void)
{
	struct keyring_item *del_item = 0;
	int bkt = 0;

	printk(KERN_DEBUG "[%s] %s", KEYRINGCTL_DEVICE_NAME, __func__);

	// hash_del & device_destroy
	hash_for_each(keyring_table, bkt, del_item, hash) {
		kfree(del_item->data);
		hash_del(&del_item->hash);
		device_destroy(keyring_class, MKDEV(keyring_device_major, del_item->key));
	}

	// device_destroy
	device_destroy(keyringctl_class, MKDEV(keyringctl_device_major, 0));

	// class_destroy
	class_destroy(keyring_class);
	class_destroy(keyringctl_class);

	// unregister_chrdev
	unregister_chrdev(keyring_device_major, KEYRING_DEVICE_NAME);
	unregister_chrdev(keyringctl_device_major, KEYRINGCTL_DEVICE_NAME);
}

module_init(keyring_module_init);
module_exit(keyring_module_exit);

MODULE_AUTHOR("wonyong <parkour0071@gmail.com>");
MODULE_DESCRIPTION("A Simple keyring driver");
MODULE_LICENSE("GPL v2");
