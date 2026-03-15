.PHONY: all kernel clean format

all: kernel

kernel:
	@echo "Building Kernel..."
	$(MAKE) -C kernel

format:
	@echo "Formatting Kernel..."
	$(MAKE) -C kernel format

clean:
	@echo "Cleaning Kernel..."
	$(MAKE) -C kernel clean
