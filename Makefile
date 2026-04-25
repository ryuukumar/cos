.PHONY: all kernel clean format user initramfs

all: kernel user initramfs

kernel:
	@echo "Building Kernel..."
	$(MAKE) -C kernel

user:
	@echo "Building userspace..."
	$(MAKE) -C user

initramfs: user
	@( cd build/initramfs && find . -print | cpio -o -H newc -v ) > build/initramfs.cpio

format:
	@echo "Formatting Kernel..."
	$(MAKE) -C kernel format
	@echo "Formatting User..."
	$(MAKE) -C user format

clean:
	@echo "Cleaning Kernel..."
	$(MAKE) -C kernel clean
	@echo "Cleaning User..."
	$(MAKE) -C user clean
