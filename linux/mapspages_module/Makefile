KDIR ?= ..           # use .. for folder inside the kernel sources -OR-
KDIR ?= ../linux     # use ../linux for folder outside the tree

kbuild:
	make -C $(KDIR) M=`pwd` modules
	mkdir -p /lib/modules/5.4.0-custom/extra
	rm -f /lib/modules/5.4.0-custom/extra/mapspages_module.ko
	cp /data/ubuntu/linux/mapspages_module/mapspages_module.ko /lib/modules/5.4.0-custom/extra/
	depmod
	modprobe -r mapspages_module
clean:
	make -C $(KDIR) M=`pwd` clean
