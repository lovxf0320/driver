obj-m+=led_dev.o 
obj-m+=led_drv.o
KERNEL_DIR:=/home/gec/6818GEC/kernel
CROSS_COMPILE :=/home/gec/6818GEC/prebuilts/gcc/linux-x86/arm/arm-eabi-4.8/bin/arm-eabi-
PWD:=$(shell pwd)

default:
	$(MAKE) arch=ARM CROSS_COMPILE=$(CROSS_COMPILE) -C $(KERNEL_DIR) M=$(PWD) modules
	make clean
	cp *.ko /home/gec/nfs
	arm-linux-gcc led_test.c -o led_test
	cp led_test /home/gec/nfs
clean:
	rm  *.o *.order .*.cmd  *.mod.c *.symvers .tmp_versions -rf
