#include <linux/moudle.h>
#include <linux/init.h>
//�����˳�ʼ�����������ı�Ǻ� __init��__exit�����ڽ���Щ�����ŵ��ض��ĽڶΣ��Ա��ڳ�ʼ��������ڴ档
#include <linux/cdev.h>
//�ṩ�ַ��豸��struct cdev����صĽӿں����������� cdev_init()��cdev_add()��cdev_del() �ȡ�
#include <linux/fs.h>
//�������ļ�ϵͳ���ĵ����ݽṹ��ӿڣ�����Ҫ���� struct file_operations���ļ������ӿڣ��͸����豸�Ź�������

/*struct cdev {
	struct kobject kobj;
	struct module *owner;
	const struct file_operations *ops;
	struct list_head list;
	dev_t dev;
	unsigned int count;
};*/
static struct cdev led_cdev;

//typedef __u32 __kernel_dev_t;
//typedef __kernel_dev_t  dev_t;
//����������һ���豸�ţ������豸�Ŵ��������ͣ�����ʼ��Ϊ0
static dev_t led_dev_num=0;

//Ӧ�ó���ִ��open���ͻ�ִ�д˺���
static int led_open(struct inode *inode,struct file *file){
/* struct inode:
 * Keep mostly read-only and often accessed (especially for
 * the RCU path lookup and 'stat' data) fields at the beginning
 * of the 'struct inode'
 * �����ڵ�ṹ��
 * �Ѵ������ֻ���ģ����Ҿ��������ʵ��ֶΣ��ر����� RCU ·�����Һ� `stat` ϵͳ������ʹ�õ����ݣ�
 * ���ڽṹ�忪ͷ
 */
 
 /*
 * fu_list becomes invalid after file_free is called and queued via
 * fu_rcuhead for RCU freeing
 */
 /*
 * fu_list�����Ч����file_free�����ú���ͨ��fu_rcuhead�Ŷӵȴ�RCU�ͷ�
 */
	printk("<4>""led_open");

	return 0;

}

int led_close(struct inode *inode,struct file *file){
	printk("<4>""led_close");

	return 0;
}

/* file_operations
 * These macros are for out of kernel modules to test that
 * the kernel supports the unlocked_ioctl and compat_ioctl
 * fields in struct file_operations. */

static struct file_operations led_fops={
	//#define THIS_MODULE ((struct module *)0)
	.owner = THIS_MODULE,
	.open = led_open,
	.release = led_close,
}

//�������
static int __init led_init(void){
	//__init��Ǹ��߱���������������.init.text����ʼ����Ϻ�ɻ���
	int rt = 0;

	//�����豸��
	//#define MKDEV(ma,mi)	(((ma) << MINORBITS) | (mi))
	//#define MINORBITS	20
	led_dev_num=MKDEV(239,0);
	
	//��̬ע���豸
	//���豸����Ч������/proc/devices�ļ����ҵ�"myled"��ֻ�Ǽ���������Щ�豸
	/**
	 * int register_chrdev_region(dev_t from, unsigned count, const char *name)
	 * register_chrdev_region() - register a range of device numbers
	 * @from: the first in the desired range of device numbers; must include
	 *        the major number.
	 * @count: the number of consecutive device numbers required
	 * @name: the name of the device or driver.
	 *
	 * Return value is zero on success, a negative error code on failure.
	 */
	rt = register_chardev_region(led_dev_num,1,"myled");
	if(rt<0){
		printk("<3>""err_register_chardev_region \n");

		goto err_register_chardev_region;
	}
	/**
	 * void cdev_init(struct cdev *cdev, const struct file_operations *fops)
	 * cdev_init() - initialize a cdev structure
	 * @cdev: the structure to initialize
	 * @fops: the file_operations for this device
	 *
	 * Initializes @cdev, remembering @fops, making it ready to add to the
	 * system with cdev_add().
	 */
	cdev_init(&led_cdev,&led_fops);

	/**
	 * int cdev_add(struct cdev *p, dev_t dev, unsigned count)
	 * cdev_add() - add a char device to the system
	 * @p: the cdev structure for the device
	 * @dev: the first device number for which this device is responsible
	 * @count: the number of consecutive minor numbers corresponding to this
	 *         device
	 *
	 * cdev_add() adds the device represented by @p to the system, making it
	 * live immediately.  A negative error code is returned on failure.
	 */
    //���ַ��豸�����ں�
	rt=cdev_add(led_cdev,led_cdev_num,1);
	if(rt<0){
		printk("<3>""err_cdev_add \n");

		goto err_cdev_add;
	}

	printk("Hello lxf\n");

	return 0;


	err_register_chardev_region:
		return rt;
	err_cdev_add:
	/**
	 * void unregister_chrdev_region(dev_t from, unsigned count)
	 * unregister_chrdev_region() - return a range of device numbers
	 * @from: the first in the range of numbers to unregister
	 * @count: the number of device numbers to unregister
	 *
	 * This function will unregister a range of @count device numbers,
	 * starting with @from.  The caller should normally be the one who
	 * allocated those numbers in the first place...
	 */
		unregister_chardev_region(led_dev_num,1);

}

static void __exit led_exit(void){
/**
 * ���ں����Ƴ�
 * void cdev_del(struct cdev *p)
 * cdev_del() - remove a cdev from the system
 * @p: the cdev structure to be removed
 *
 * cdev_del() removes @p from the system, possibly freeing the structure
 * itself.
 */

	cdev_del(&led_cdev);
	unregister_chardev_region(led_dev_num,1);
	printk("GoodBye lxf");
}


module_init(led_init);

module_exit(led_exit);


MODULE_AUTHOR("lixiaofan");
MODULE_DESCRIPTION("Led Device Driver");
MODULE_LICENSE("GPL");	



























