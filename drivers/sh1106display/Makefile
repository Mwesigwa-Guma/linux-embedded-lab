obj-m += sh1106.o

build:
	make -C /home/pi/linux-headers-$(shell uname -r) modules M=$(PWD)

clean:
	make -C /home/pi/linux-headers-$(shell uname -r) clean M=$(PWD)

load:
	sudo insmod sh1106.ko
