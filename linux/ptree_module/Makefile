KDIR ?= ..           # use .. for folder inside the kernel sources -OR-
KDIR ?= ../linux     # use ../linux for folder outside the tree

kbuild:
	make -C $(KDIR) M=`pwd`
	mkdir -p /lib/modules/5.4.0-custom/extra
	rm -f /lib/modules/5.4.0-custom/extra/ptree_module.ko
	cp /data/ubuntu/linux/ptree_module/ptree_module.ko /lib/modules/5.4.0-custom/extra/
	depmod
	modprobe -r ptree_module
clean:
	make -C $(KDIR) M=`pwd` clean
