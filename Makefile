obj-m += gpio_led_driver.o
obj-m += gpio_button_driver.o
obj-m += ioctl.o


KDIR := /home/wings/buildroot/output/build/linux-custom
CROSS_COMPILE := /home/wings/buildroot/output/host/bin/arm-buildroot-linux-gnueabihf-
ARCH := arm
M := $(shell pwd)

all:
	make -C $(KDIR) M=$(M) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) modules

clean:
	make -C $(KDIR) M=$(M) clean
