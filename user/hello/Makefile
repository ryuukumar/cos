# This makefile is quite similar to the one used by the kernel, with
# the exception of some kernel-only flags that have been omitted here

override MAKEFLAGS += -rR
override TARGET := hello

define DEFAULT_VAR =
	ifeq ($(origin $1),default)
		override $(1) := $(2)
	endif
	ifeq ($(origin $1),undefined)
		override $(1) := $(2)
	endif
endef

override DEFAULT_CC := x86_64-elf-gcc
$(eval $(call DEFAULT_VAR,CC,$(DEFAULT_CC)))

override DEFAULT_LD := x86_64-elf-ld
$(eval $(call DEFAULT_VAR,LD,$(DEFAULT_LD)))

override DEFAULT_CFLAGS := -g -O2 -pipe
$(eval $(call DEFAULT_VAR,CFLAGS,$(DEFAULT_CFLAGS)))
 
# User controllable C preprocessor flags. We set none by default.
override DEFAULT_CPPFLAGS :=
$(eval $(call DEFAULT_VAR,CPPFLAGS,$(DEFAULT_CPPFLAGS)))
 
# User controllable linker flags. We set none by default.
override DEFAULT_LDFLAGS :=
$(eval $(call DEFAULT_VAR,LDFLAGS,$(DEFAULT_LDFLAGS)))
 
override SYSROOT := $(abspath ../../initramfs)

# Internal C flags that should not be changed by the user.
override CFLAGS += \
	-Wall \
	-Wextra \
	-Wstrict-prototypes \
	-Wmissing-prototypes \
	-Wmissing-declarations \
	-std=gnu23 \
	-ffreestanding \
	-fno-stack-protector \
	-fno-stack-check \
	-fno-lto \
	-fno-PIE \
	-fno-PIC \
	-m64 \
	-march=x86-64 \
	-mabi=sysv
 
# Internal C preprocessor flags that should not be changed by the user.
override CPPFLAGS := \
	-I $(SYSROOT)/usr/include \
	-I include \
	-I. \
	$(CPPFLAGS) \
	-MMD \
	-MP
 
# Internal linker flags that should not be changed by the user.
override LDFLAGS += \
	-L$(SYSROOT)/usr/lib \
	-nostdlib \
	-static \
	-m elf_x86_64 \
	-z max-page-size=0x1000

ifeq ($(shell $(LD) --help 2>&1 | grep 'no-pie' >/dev/null 2>&1; echo $$?),0)
	override LDFLAGS += -no-pie
endif

override CFILES := $(shell find -L . -type f -name '*.c')
override ASFILES := $(shell find -L . -type f -name '*.s')

override BUILD_DIR := ../../build/user/bin
override INITRAMFS_DIR := $(SYSROOT)/bin

override OBJ := $(patsubst %.c,$(BUILD_DIR)/%.c.o,$(CFILES)) $(patsubst %.s,$(BUILD_DIR)/%.s.o,$(ASFILES))
override HEADER_DEPS := $(patsubst %.c,$(BUILD_DIR)/%.c.d,$(CFILES)) $(patsubst %.s,$(BUILD_DIR)/%.s.d,$(ASFILES))
 
.PHONY: all
all: $(BUILD_DIR)/$(TARGET)

$(BUILD_DIR)/$(TARGET): $(OBJ)
	@printf "LD\t%s\n" $@
	@$(LD) $(SYSROOT)/usr/lib/crt0.o $(OBJ) $(LDFLAGS) -lc -lcos -o $@
	@mkdir -p $(INITRAMFS_DIR)
	@cp $(BUILD_DIR)/$(TARGET) $(INITRAMFS_DIR)

-include $(HEADER_DEPS)

$(BUILD_DIR)/%.c.o: %.c
	@mkdir -p $(@D)
	@printf "CC\t%s\n" $<
	@$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

$(BUILD_DIR)/%.s.o: %.s
	@mkdir -p $(@D)
	@printf "CC\t%s\n" $<
	@$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)

.PHONY: format
format:
	find . -type f \( -name '*.c' -o -name '*.h' \) ! -name 'limine.h' -exec clang-format -i {} +
