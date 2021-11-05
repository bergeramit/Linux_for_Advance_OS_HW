KDIR ?= ..           # use .. for folder inside the kernel sources -OR-
KDIR ?= ../linux     # use ../linux for folder outside the tree

kbuild:
	make -C $(KDIR) M=`pwd`
	cp ./ptree_module.ko /lib/modules/5.4.0-custom+/extra/ptree_module.ko
	depmod
clean:
	make -C $(KDIR) M=`pwd` clean
