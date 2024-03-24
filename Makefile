#obj-m += miModulo.o
#obj-m += charDev.o
obj-m += charDevDocu.o


all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
