.PHONY: help

GCC := "/home/baptiste/opt/cross/bin/i686-elf-gcc"
GDB := "/usr/bin/gdb"
LD := "/home/baptiste/opt/cross/bin/i686-elf-ld"
QEMU := "/usr/bin/qemu-system-i386"
GMKR := "/usr/bin/grub-mkrescue"

help: ## Show this help
	@grep -E '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | sort | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-30s\033[0m %s\n", $$1, $$2}'

kernel: clean ## Compiles the kernel
	$(GCC) -nostdlib -nostdinc -ffreestanding -c -o kernel.o kernel.c
	$(GCC) -nostdlib -nostdinc -ffreestanding -c -o boot.o boot.S
	$(LD) -T linker.ld -o kernel boot.o kernel.o

iso: clean kernel ## Creates the bootable iso
	mkdir iso
	mkdir iso/boot
	mkdir iso/boot/grub
	cp kernel iso/boot/
	cp grub.cfg iso/boot/grub/
	$(GMKR) -o image.iso iso

test: iso ## Boots the iso
	$(QEMU) image.iso

debug: iso ## Run a debug session
	$(QEMU) -s -S image.iso &
	$(GDB) kernel \
		-ex "target remote localhost:1234"

clean: ## Removes all the sh**
	rm -rf iso kernel image.iso boot.o kernel.o
