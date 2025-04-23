
lcd:
	dtc -@ -I dts -O dtb -o lcd1602.dtbo lcd1602.dts

oled:
	dtc -@ -I dts -O dtb -o sh1106.dtbo sh1106.dts

tofirmware:
	sudo cp *.dtbo /boot/firmware/overlays/

removeoverlay:
	sudo rm /boot/firmware/overlays/sh1106*

checkoverlay:
	sudo ls /boot/firmware/overlays/ | grep sh1106

clean:
	rm *.dtbo
