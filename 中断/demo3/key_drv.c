#include <linux/init.h>
#include <linux/module.h>
#include <mach/devices.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>

/*typedef irqreturn_t (*irq_handler_t)(int irq, void *dev_id);
��ζ������ں�ͷ�ļ��У�include/linux/interrupt.h
| ����       | ����       | ˵��                                             |
| -------- | -------- | ---------------------------------------------- |
| `irq`    | `int`    | ��ʾ�����жϵ��жϺţ����ں˴��룩                              |
| `dev_id` | `void *` | ͨ���Ǵ��� `request_irq()` �����һ�����������ڴ����豸���ݻ�˽������ָ�롣 |

���������ں˶����ö�����ͣ����ڱ�ʾ�жϴ��������������£�
typedef enum {
    IRQ_NONE,     // û�д�������ж�
    IRQ_HANDLED,  // �ɹ�����������ж�
    IRQ_WAKE_THREAD  // �����̻߳��жϣ������ã�
} irqreturn_t;
*/

static unsigned int data = 100;

static irqreturn_t mykey_irq_handler(int irq, void *dev_id){
	unsigned int d = *(unsigned int*)dev_id;
	
	printk(KERN_INFO"mykey_irq_handler,%d",d);
	return IRQ_HANDLED;
}


static int __init mykey_init(void)
{
	int rt=0;
	rt=request_irq(
			gpio_to_irq(PAD_GPIO_A+28),	// �� GPIO ���תΪ�жϺ�
			mykey_irq_handler,			// �жϷ�����
			IRQF_TRIGGER_FALLING,		// �½��ش������������ͣ�
			"gpio_28_irq",				// /proc/interrupts ����ʾ������	
			&data						//�������ݸ��жϷ�����(Ҫ��һ��ָ�룬��һ����ַ)
	);
/*	| �곣��                  | ����                     | ����                    |
	| ----------------------- | ------------------------ | ----------------------- |
	| `IRQF_TRIGGER_RISING`   | �����أ��� �� �ߣ�        | �ɿ��������źŻָ�      |
	| `IRQF_TRIGGER_FALLING`  | �½��أ��� �� �ͣ�        | ���°������ź�����      |
	| `IRQF_TRIGGER_HIGH`     | �ߵ�ƽ����ʱ����         | �����ã����� SoC ������ |
	| `IRQF_TRIGGER_LOW`      | �͵�ƽ����ʱ����         | ĳЩ���������          |
	| \`IRQF\_TRIGGER\_RISING | IRQF\_TRIGGER\_FALLING\` | ˫���ش��������º�̧����Ӧ |*/

	if(rt<0){
		printk(KERN_INFO"request_irq_GPIO_A+28 fail\n");
		goto err_request_irq;
	}

	printk(KERN_INFO"mykey_init\n");
	return 0;
	
err_request_irq:
	return rt;
}


static void __exit mykey_exit(void){
	free_irq(gpio_to_irq(PAD_GPIO_A+28),NULL);
	printk(KERN_INFO"mykey_exit\n");
}
a

module_init(mykey_init);//��������ڣ�ʹ��insmod������ظ�����
module_exit(mykey_exit);//�����ĳ��ڣ�ʹ��rmmod����ж�ظ�����

MODULE_AUTHOR("li xiaofan");
MODULE_DESCRIPTION("This is key driver");
MODULE_LICENSE("GPL");