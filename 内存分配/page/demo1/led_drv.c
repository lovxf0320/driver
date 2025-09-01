#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/string.h>

static unsigned char *p=NULL; 	//�����洢��̬������ڴ��ַ
static unsigned long addr;

static int __init myled_init(void){
	unsigned long addr;
	printk(KERN_INFO"myled_init\n");
	
	addr=__get_free_page(GFP_KERNEL);
	/*
	GFP_KERNEL����ʾ�����������������ں��ڴ���䡣
	GFP_ATOMIC����ʾ�����ڴ�ʱ�����������������ж������ġ�
	GFP_DMA����ʾ������ڴ�������� DMA ������
	*/
	
	p=(unsigned char*)addr;
	if(p==NULL){
		printk(KERN_INFO"__get_free_page fail\n");
		return -ENOMEM;			// �����ڴ����ʧ�ܵĴ�����
	}
	
	strcpy(p,"This is lixiaofan");
	
	printk(KERN_INFO"p=%s\n",p);
	
	printk(KERN_INFO"p virt_addr:0x%p phy_addr:%lx\n",p,(unsigned long)virt_to_phys(p));
	//virt_to_phys(p)���������ַ p ת��Ϊ�����ַ
	
	return 0;
}

static void __exit myled_exit(void){
	free_page(addr);				 // �ͷŷ�����ڴ�
	printk("myled_exit\n");  // ��ӡж����Ϣ
}


module_init(myled_init);//��������ڣ�ʹ��insmod������ظ�����
module_exit(myled_exit);//�����ĳ��ڣ�ʹ��rmmod����ж�ظ�����

MODULE_AUTHOR("LI XIAOFAN");
MODULE_DESCRIPTION("This is led driver");
MODULE_LICENSE("GPL");