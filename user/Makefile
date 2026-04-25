.PHONY: all hello kclib

all: hello kclib

lib: kclib

bin: hello

kclib:
	@echo "Building kclib..."
	$(MAKE) -C kclib

hello:
	@echo "Building hello..."
	$(MAKE) -C hello

format:
	@echo "Formatting userspace..."
	$(MAKE) -C hello format
	$(MAKE) -C kclib format

clean:
	@echo "Cleaning userspace..."
	$(MAKE) -C hello clean
	$(MAKE) -C kclib clean