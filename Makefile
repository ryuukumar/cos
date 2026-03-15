.PHONY: all kernel clean

all: kernel

kernel:
	@echo "Building Kernel..."
	$(MAKE) -C kernel

clean:
	@echo "Cleaning Kernel..."
	$(MAKE) -C kernel clean
