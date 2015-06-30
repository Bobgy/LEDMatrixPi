obj-m := char_device.o

# this is a usual location
# uncomment this to replace my custom location
# KERNELDIR ?= /lib/modules/$(shell uname -r)/build
KERNELDIR ?= $(shell cat .env)

PWD := $(shell pwd)

OPTIONS := ARCH=arm CROSS_COMPILE=/usr/bin/arm-linux-gnueabi-

all:
	$(MAKE) $(OPTIONS) -C $(KERNELDIR) M=$(PWD) modules

clean:
	rm -f *.o *.ko modules.order Module.symvers char_device.mod.c

deploy:
	scp -r ./ pi@192.168.9.1:~/module/char-device-driver/

insmod:
	sudo rmmod char_device
	sudo insmod char_device.ko
