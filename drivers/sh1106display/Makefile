obj-m += sh1106.o

build:
	make -C /lib/modules/$(shell uname -r)/build modules M=$(PWD)

clean:
	make -C /lib/modules/$(shell uname -r)/build clean M=$(PWD)

load:
	sudo modprobe regmap-spi
	sudo insmod sh1106.ko

remove:
	sudo rmmod sh1106

check:
	sudo lsmod | grep sh1106
