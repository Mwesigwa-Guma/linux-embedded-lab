
build:
	dtc -@ -I dts -O dtb -o lcd1602.dtbo lcd1602.dts

cptofirmware:
	sudo cp lcd1602a.dtbo /boot/firmware/overlays/

clean:
	rm *.dtbo
