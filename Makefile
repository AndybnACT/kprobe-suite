KDIR=/source/linux-4.4.x/

obj-m += myprobes.o
myprobes-y := kprobe_example.o

ccflags-y := -std=gnu99 -Wno-declaration-after-statement -I $(KDIR)drivers/nvme/host -I $(KDIR)

.PHONY: all clean

all:
	make -C $(KDIR)  M=$(PWD) modules

clean:
	make -C $(KDIR) M=$(PWD) clean
# DO NOT DELETE
