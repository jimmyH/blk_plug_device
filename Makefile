# If KERNELRELEASE is defined, we've been invoked from the
# kernel build system and can use its language.

ifneq ($(KERNELRELEASE),)
	obj-m := blk_plug_device.o

# Otherwise we were called directly from the command
# line; invoke the kernel build system.
else
	KERNELDIR ?= /usr/src/linux-headers-`uname -r`
	PWD  := $(shell pwd)

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules


clean:
	rm -f *.o *.ko *.mod.c modules.order Module.symvers .*.o.cmd .*.ko.cmd

endif

