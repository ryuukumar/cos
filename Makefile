.PHONY: all lib kernel clean format user initramfs

all: lib kernel user initramfs

kernel:
	@echo "Building Kernel..."
	$(MAKE) -C kernel

user: user_bin

user_bin:
	@echo "Building user binaries..."
	$(MAKE) -C user bin

lib:
	$(MAKE) -C lib

initramfs: user
	@( cd build/initramfs && find . -print | cpio -o -H newc -v ) > build/initramfs.cpio

format:
	@echo "Formatting Kernel..."
	$(MAKE) -C kernel format
	@echo "Formatting Lib..."
	$(MAKE) -C lib format
	@echo "Formatting User..."
	$(MAKE) -C user format

clean:
	@echo "Cleaning Kernel..."
	$(MAKE) -C kernel clean
	@echo "Cleaning User..."
	$(MAKE) -C user clean
