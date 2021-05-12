KDIR=/path/to/linux/
CURDIR=$(PWD)

obj-m += myprobes.o
myprobes-y := kprobe.o

ccflags-y := -std=gnu99 -Wno-declaration-after-statement -I $(KDIR) 
ldflags-y += -T $(CURDIR)/linker.ld

.PHONY: all clean

all:
	make -C $(KDIR)  M=$(PWD) modules

clean:
	make -C $(KDIR) M=$(PWD) clean

