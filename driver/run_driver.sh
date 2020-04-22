make

rm -r .tmp_versions .2261.tmp .vga_driver.ko.cmd .vga_driver.mod.o.cmd .vga_driver.o.cmd \
	Module.symvers modules.order vga_driver.mod.c vga_driver.mod.o vga_driver.o

rmmod vga_driver.ko
insmod vga_driver.ko
