obj-m := nugpgpu.o

bin        := ./bin
mackerel-s := mackerel
device-s   := ./device
generated   := ./generated

# C Source files
# Add C files here <space> separated
nugpgpu-objs := source/nugpgpu_init.o
nugpgpu-objs += include/mackerel.o	# TODO: Find a better way to do it.
nugpgpu-objs += source/nugpgpu_dbg.o
nugpgpu-objs += source/nugpgpu_memmgr.o
nugpgpu-objs += source/nugpgpu_gtt.o
nugpgpu-objs += source/nugpgpu_regrw.o
nugpgpu-objs += source/nugpgpu_ringbuffer.o
nugpgpu-objs += source/nugpgpu_tests.o

KERNEL = $(shell uname -r)

all: udev mackerel generate module

folders:
	mkdir -p $(bin)
	mkdir -p $(generated)

module: udev mackerel generate
	make -C /lib/modules/$(KERNEL)/build M=$(PWD) modules

udev:
	sudo su -c 'echo "KERNEL==\"nugpgpu\", MODE=\"0666\"" > /etc/udev/rules.d/50-nugpgpu.rules'

mackerel: folders $(mackerel-s)/
	ghc -i./$(mackerel-s) -odir $(bin) -hidir $(bin) -rtsopts=all --make $(mackerel-s)/Main.hs -o $(bin)/mackerel

generate: folders $(device-s)/ mackerel 
	$(bin)/mackerel --shift-driver -c $(device-s)/nugpgpu.dev -o $(generated)/nugpgpu_dev.h

clean-module:
	make -C /lib/modules/$(KERNEL)/build M=$(PWD) clean

clean:
	rm -rf $(bin)
	rm -rf $(generated)
	make -C /lib/modules/$(KERNEL)/build M=$(PWD) clean
