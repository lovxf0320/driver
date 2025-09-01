#include <linux/init.h>         // ����ģ���ʼ�����˳���
#include <linux/module.h>       // �ṩģ����غ꣬�� module_init() �� module_exit()
#include <mach/devices.h>       // ��Ӳ��ƽ̨��ص��豸����
#include <linux/interrupt.h>    // �жϴ�����غ���
#include <linux/gpio.h>         // GPIO ��������
#include <linux/input.h>        // �����豸�����簴������������

static struct input_dev* button_dev;
/*
input_dev������һ���ṹ�壬������һ�������豸��
ÿ�������豸���簴���������������ȣ�����һ�� input_dev �ṹ�壬
���������豸�����ԣ�����֧�ֵ��¼����͡��������͵ȡ�
*/
static irqreturn_t mykey_irq_handler(int irq, void *dev_id){
	
	/* ���ں˻㱨����״̬ */
	input_report_key(button_dev,KEY_ENTER,!gpio_get_value(PAD_GPIO_A));
	/* 
	button_dev��ָ�������豸��ָ�롣
	KEY_ENTER����ʾ���Ǳ�����ǡ��س��������¼�������Ը���Ӳ����ѡ���Ӧ�ļ�ֵ��
	!gpio_get_value(PAD_GPIO_A+28)����ȡ GPIO ���ŵĵ�ƽ״̬���͵�ƽ��0����ʾ���£�
	��ת�󱨸�Ϊ���£�1�����ɿ���0���� 
	*/
	
	/* �㱨���� */
	input_sync(button_dev);
	
	return IRQ_HANDLED;
}

static int __init mykey_init(void){
	int rt=0;
	rt=request_irq(
			gpio_to_irq(PAD_GPIO_A+28),	// �� GPIO ���תΪ�жϺ�
			mykey_irq_handler,			// �жϷ�����
			IRQF_TRIGGER_FALLING,		// �½��ش������������ͣ�
			"gpio_28_irq",				// /proc/interrupts ����ʾ������	
			NULL						// �������豸�ṹ��
	);
	if(rt<0){
		printk(KERN_INFO"request_irq_GPIO_A+28 fail\n");
		goto err_request_irq;
	}
/*	| �곣��                  | ����                     | ����                    |
	| ----------------------- | ------------------------ | ----------------------- |
	| `IRQF_TRIGGER_RISING`   | �����أ��� �� �ߣ�        | �ɿ��������źŻָ�      |
	| `IRQF_TRIGGER_FALLING`  | �½��أ��� �� �ͣ�        | ���°������ź�����      |
	| `IRQF_TRIGGER_HIGH`     | �ߵ�ƽ����ʱ����         | �����ã����� SoC ������ |
	| `IRQF_TRIGGER_LOW`      | �͵�ƽ����ʱ����         | ĳЩ���������          |
	| \`IRQF\_TRIGGER\_RISING | IRQF\_TRIGGER\_FALLING\` | ˫���ش��������º�̧����Ӧ |*/


	/* Ϊ�����豸�����ڴ�ռ�
	ʹ�� input_allocate_device() ��������һ�� input_dev �ṹ�壬
	����ṹ����������ǵ������豸��	*/
	button_dev=input_allocate_device();
	if(!button_dev){
		printk(KERN_ERR "Not enough memory\n");
		rt = -ENOMEM;
		goto err_input_allocate_device;
	}


	/* ���������豸���¼����͡����루֧����Щ������ */
	set_bit(EV_KEY,button_dev->evbit);

	/* ����֧�ֵİ��� */
	set_bit(KEY_ENTER,button_dev->keybit);
	set_bit(KEY_UP,button_dev->keybit);
	set_bit(KEY_DOWN,button_dev->keybit);	
	set_bit(KEY_LEFT,button_dev->keybit);	
	set_bit(KEY_RIGHT,button_dev->keybit);	
	set_bit(KEY_Z,button_dev->keybit);
	set_bit(KEY_X,button_dev->keybit);	
	
	
	button_dev->name="mykey_input";
	button_dev->id.bustype=0x0000;
	button_dev->id.vendor =0x1688;	
	button_dev->id.product=0x6666;		
	button_dev->id.version=0x1001;	

	/* ע�������豸 */
    rt = input_register_device(button_dev);
    if (rt) {
        printk(KERN_ERR "Failed to register device\n");
        goto err_input_register_device;
    }

	printk(KERN_INFO"mykey_init\n");
	return 0;
	
err_input_register_device:
	input_free_device(button_dev);
	
err_input_allocate_device:
	free_irq(gpio_to_irq(PAD_GPIO_A+28),NULL);
	
err_request_irq:
	return rt;
}

static void __exit mykey_exit(void){
	/* ע�������豸 */
    input_unregister_device(button_dev);

    /* �ͷ������豸������ڴ� */
    input_free_device(button_dev);

    /* �ͷ��ж� */
    free_irq(gpio_to_irq(PAD_GPIO_A+28), NULL);

	printk(KERN_INFO"mykey_exit\n");
}


module_init(mykey_init);//��������ڣ�ʹ��insmod������ظ�����
module_exit(mykey_exit);//�����ĳ��ڣ�ʹ��rmmod����ж�ظ�����

MODULE_AUTHOR("li xiaofan");
MODULE_DESCRIPTION("This is key driver");
MODULE_LICENSE("GPL");