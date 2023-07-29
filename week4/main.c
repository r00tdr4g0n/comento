#include <linux/module.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/spinlock.h>
#include <linux/hashtable.h>

#define KEYRINGCTL_DEVICE_NAME				"keyringctl"
#define KEYRING_DEVICE_NAME					"keyring"
#define KEYRING_CLASS_NAME					"keyring"
#define KEYRING_BUF_SIZE					32
#define KEYRING_MAGIC_NUMBER				'K'
#define KEYRING_DEVICE_IOCTL_ADD			_IOW(KEYRING_MAGIC_NUMBER, 0, unsigned int)
#define KEYRING_DEVICE_IOCTL_DEL			_IOW(KEYRING_MAGIC_NUMBER, 1, unsigned int)
#define KEYRING_DEVICE_IOCTL_SHOW   		_IO(KEYRING_MAGIC_NUMBER, 2)
#define KEYRING_HASH_SIZE					10

static DEFINE_HASHTABLE(keyring_table, KEYRING_HASH_SIZE);
static DEFINE_RWLOCK(keyring_device_rwlock);

static int 					keyring_device_major;
static int					keyring_device_minor;
static struct class			*keyring_class;
static struct device 		*keyring_device;

struct keyring_item {
	int key;
	char data[KEYRING_BUF_SIZE];
	struct hlist_node hash;
};

static int 	    keyring_device_open(struct inode *inode, struct file *fp);
static int 	    keyring_device_release(struct inode *inode, struct file *fp);
static ssize_t 	keyring_device_read(struct file *fp, char __user *buf, size_t len, loff_t *ppos);
static ssize_t 	keyring_device_write(struct file *fp, const char __user *buf, size_t len, loff_t *ppos);
static long     keyring_device_ioctl(struct file *fp, unsigned int cmd, unsigned long arg);
int             create_keyring_device(unsigned int num);
void            delete_keyring_device(unsigned int num);
void            add_item(int key);
char*           get_item(int key);

static struct   file_operations keyring_device_fops = {
	.open 		    = keyring_device_open,
	.release 	    = keyring_device_release,
	.read		    = keyring_device_read,
	.write		    = keyring_device_write,
    .unlocked_ioctl = keyring_device_ioctl,
};

void add_item(int key)
{
	struct keyring_item *new_item;
    new_item = kmalloc(sizeof(struct keyring_item), GFP_KERNEL);
	new_item->key = key;
	memset(new_item->data, 0, sizeof(new_item->data));
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

static int keyring_device_open(struct inode *inode, struct file *fp)
{
	keyring_device_minor = iminor(inode);
	printk(KERN_DEBUG "[%s] %s - minor : %d", KEYRING_DEVICE_NAME, __func__, keyring_device_minor);
	return 0;
}

static int keyring_device_release(struct inode *inode, struct file *fp)
{
    keyring_device_minor = iminor(inode);
	printk(KERN_DEBUG "[%s] %s - minor : %d", KEYRING_DEVICE_NAME, __func__, keyring_device_minor);
	return 0;
}

static ssize_t keyring_device_read(struct file *fp, char __user *buf, size_t len, loff_t *ppos)
{
	int 	written_bytes = 0;
	char*	key_data = 0; 

    if (keyring_device_minor == 0) return -EINVAL;

	printk(KERN_DEBUG "[%s] %s - minor : %d", KEYRING_DEVICE_NAME, __func__, keyring_device_minor);

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

    if (keyring_device_minor == 0) return -EINVAL;

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

static long keyring_device_ioctl(struct file *fp, unsigned int cmd, unsigned long arg)
{
	int 		ret = 0;
	int			num = *(int *)arg;

    if (_IOC_TYPE(cmd) != KEYRING_MAGIC_NUMBER) return -EINVAL;

    if (keyring_device_minor) return -EINVAL;

    if (!arg) {
        printk(KERN_ERR "Invalid value entered, 0 is not allowed.");
        return -1;
    }

	switch (cmd) {
		case KEYRING_DEVICE_IOCTL_ADD:
    		ret = create_keyring_device(num);	
			break;
		case KEYRING_DEVICE_IOCTL_DEL:
			delete_keyring_device(num);
			break;
		case KEYRING_DEVICE_IOCTL_SHOW:
			printk(KERN_DEBUG "[%s] SHOW", KEYRING_DEVICE_NAME);
			break;
		default:
			return -EINVAL;
            break;
	}

	return ret;
}

int create_keyring_device(unsigned int num)
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
    
	add_item(num);
	printk(KERN_DEBUG "[%s] Add keyring%d", KEYRING_DEVICE_NAME, num);

	return 0;
}

void delete_keyring_device(unsigned int num)
{
	struct keyring_item *del_item = 0;

	hash_for_each_possible(keyring_table, del_item, hash, num) {
		hash_del(&del_item->hash);
	}

	device_destroy(keyring_class, MKDEV(keyring_device_major, num));

    printk(KERN_DEBUG "[%s] Delete keyring%d", KEYRING_DEVICE_NAME, num);
}

static int __init keyring_module_init(void)
{
	int ret = 0;
	int minor = 0;

	printk(KERN_DEBUG "[%s] %s", KEYRING_DEVICE_NAME, __func__);
	
	keyring_device_major = register_chrdev(0, KEYRING_DEVICE_NAME, &keyring_device_fops);

	if (0 > keyring_device_major)  {
		goto err_register;
	}

    keyring_class = class_create(THIS_MODULE, KEYRING_CLASS_NAME);

	if (IS_ERR(keyring_class)) {
		goto err_class;
	}

    keyring_device = device_create(
					keyring_class,
					NULL,
					MKDEV(keyring_device_major, minor),
					NULL,
					"%s", KEYRINGCTL_DEVICE_NAME);

    if (IS_ERR(keyring_device)) {
        goto err_device;
    }

	return ret;

err_device:
    printk(KERN_ERR "[%s] Failed to create device", KEYRING_DEVICE_NAME);
	class_destroy(keyring_class);
	ret = PTR_ERR(keyring_device);

err_class:
    printk(KERN_ERR "[%s] Failed to create class", KEYRING_DEVICE_NAME);		
	unregister_chrdev(keyring_device_major, KEYRING_DEVICE_NAME);
    ret = PTR_ERR(keyring_class);

err_register:
	return ret;
}

static void __exit keyring_module_exit(void)
{
	struct keyring_item *del_item = 0;
	int bkt = 0;

	printk(KERN_DEBUG "[%s] %s", KEYRINGCTL_DEVICE_NAME, __func__);

	hash_for_each(keyring_table, bkt, del_item, hash) {
		kfree(del_item->data);
		hash_del(&del_item->hash);
		device_destroy(keyring_class, MKDEV(keyring_device_major, del_item->key));
	}

	device_destroy(keyring_class, MKDEV(keyring_device_major, 0));
	class_destroy(keyring_class);
	unregister_chrdev(keyring_device_major, KEYRING_DEVICE_NAME);
}

module_init(keyring_module_init);
module_exit(keyring_module_exit);

MODULE_AUTHOR("wonyong <parkour0071@gmail.com>");
MODULE_DESCRIPTION("A Simple keyring driver");
MODULE_LICENSE("GPL v2");
