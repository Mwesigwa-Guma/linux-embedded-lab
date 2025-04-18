obj-m += lcd1602a.o

build:
	make -C /lib/modules/$(shell uname -r)/build modules M=$(PWD)

clean:
	make -C /lib/modules/$(shell uname -r)/build clean M=$(PWD)

load:
	sudo insmod lcd1602a.ko

copyto:
	scp lcd1602a.c guma@10.0.0.230:/home/guma/sources/drivers/lcddisplay/

copyfrom:
	scp guma@10.0.0.230:/home/guma/sources/drivers/lcddisplay/lcd1602a.c .
