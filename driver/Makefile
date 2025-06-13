obj-m := oxcode-driver.o
KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)
EXTRA_CFLAGS = -I$(KDIR)/include

all:
	$(MAKE) -C $(KDIR) M=$(PWD) EXTRA_CFLAGS="$(EXTRA_CFLAGS)" modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean