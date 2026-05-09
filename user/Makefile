.PHONY: all hello cosh

all: hello cosh

bin: hello cosh

hello:
	@echo "Building hello..."
	$(MAKE) -C hello

cosh:
	@echo "Building cosh..."
	$(MAKE) -C cosh 

format:
	@echo "Formatting hello..."
	$(MAKE) -C hello format
	@echo "Formatting cosh..."
	$(MAKE) -C cosh format

clean:
	@echo "Cleaning hello..."
	$(MAKE) -C hello clean