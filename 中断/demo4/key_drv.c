#include <linux/init.h>           // __init/__exit �궨��
#include <linux/module.h>         // module_init/module_exit��ģ���
#include <mach/devices.h>         // �ṩ PAD_GPIO_* ���壨ƽ̨��أ�
#include <linux/interrupt.h>      // �ж�ע�ᡢ�ͷš�������
#include <linux/gpio.h>           // GPIO �� IRQ ��غ������� gpio_to_irq��
#include <mach/platform.h>        // �ṩ PB_PIO_IRQ �궨��

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

unsigned int data1=100;
unsigned int data2=200;
unsigned int data3=300;
unsigned int data4=400;

struct gpio_irq_t{
	unsigned int  irq_n;      // �жϺ�
	irq_handler_t fun;        // �жϴ�����������ָ�룩
	unsigned long irq_f;      // �жϴ�����ʽ�����½��أ�
	const char 	  *name;      // �ж����������� /proc/interrupts
	void          *dev;       // ��ѡ��˽�в�����������������
};
static irqreturn_t mykey_irq_handler(int irq, void *dev_id);

static struct gpio_irq_t keys_irq[]={
	{PB_PIO_IRQ(PAD_GPIO_A+28),mykey_irq_handler,IRQF_TRIGGER_FALLING,"gpio_28_irq",&data1},
	{PB_PIO_IRQ(PAD_GPIO_B+30),mykey_irq_handler,IRQF_TRIGGER_FALLING,"gpio_30_irq",&data2},
	{PB_PIO_IRQ(PAD_GPIO_B+31),mykey_irq_handler,IRQF_TRIGGER_FALLING,"gpio_31_irq",&data3},
	{PB_PIO_IRQ(PAD_GPIO_B+9),mykey_irq_handler,IRQF_TRIGGER_FALLING,"gpio_9_irq",&data4},	
};

static irqreturn_t mykey_irq_handler(int irq, void *dev_id){  //irq �ǵ�ǰ�жϺ�
	int i;
	for(i=0;i<ARRAY_SIZE(keys_irq);i++){
		if(keys_irq[i].irq_n==irq){
			printk(KERN_INFO "%s trigger\n %d", keys_irq[i].name,keys_irq[i].dev);
		}
	}
	printk(KERN_INFO"mykey_irq_handler");
	return IRQ_HANDLED;
}
/*irq_handler_t��һ��typedef��typedef irqreturn_t (*irq_handler_t)(int, void*)
irqreturn_t���������ͣ���ʾ�ж��Ƿ���ɹ���
int irq���������жϵ��жϺţ����Զ����룩��
void *dev_id������ע���ж�ʱ���Ĳ���������Ϊ NULL��
printk()���ں˴�ӡ������KERN_INFO ������ͨ��Ϣ���𡱡�
IRQ_HANDLED����ʾ��ǰ�ж��ѱ��ɹ�����
*/

static int __init mykey_init(void){
	int rt=0;
	int i;
	for(i=0;i<ARRAY_SIZE(keys_irq);i++){
		rt=request_irq(
			keys_irq[i].irq_n,
			keys_irq[i].fun,
			keys_irq[i].irq_f,
			keys_irq[i].name,
			keys_irq[i].dev		
		);
		if(rt<0){
			printk(KERN_INFO"request_irq_GPIO_A+28 fail\n");
			goto err_request_irq;
		}
	}
/*	
| �곣��                  | ����                 | ����                    |
| ----------------------- | -------------------- | ----------------------- |
| `IRQF_TRIGGER_RISING`   | �����أ��� �� �ߣ�    | �ɿ��������źŻָ�      |
| `IRQF_TRIGGER_FALLING`  | �½��أ��� �� �ͣ�    | ���°������ź�����      |
| `IRQF_TRIGGER_HIGH`     | �ߵ�ƽ����ʱ����     | �����ã����� SoC ������ |
| `IRQF_TRIGGER_LOW`      | �͵�ƽ����ʱ����     | ĳЩ���������          |
| \`IRQF\_TRIGGER\_RISING | IRQF\_TRIGGER\_FALLING\` | ˫���ش��������º�̧����Ӧ |*/
	printk(KERN_INFO"mykey_init\n");
	return 0;
	
err_request_irq:
	while(i--){
		free_irq(keys_irq[i].irq_n,keys_irq[i].dev);
	}
	return rt;
}


static void __exit mykey_exit(void){
	int i=ARRAY_SIZE(keys_irq);
	while(i--){
		free_irq(keys_irq[i].irq_n,keys_irq[i].dev);
	}
	printk(KERN_INFO"mykey_exit\n");
}


module_init(mykey_init);//��������ڣ�ʹ��insmod������ظ�����
module_exit(mykey_exit);//�����ĳ��ڣ�ʹ��rmmod����ж�ظ�����

MODULE_AUTHOR("li xiaofan");
MODULE_DESCRIPTION("This is key driver");
MODULE_LICENSE("GPL");