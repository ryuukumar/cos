# Nuke built-in rules and variables.
override MAKEFLAGS += -rR
 
# This is the name that our final kernel executable will have.
# Change as needed.
override KERNEL := entry.elf
 
# Convenience macro to reliably declare user overridable variables.
define DEFAULT_VAR =
	ifeq ($(origin $1),default)
		override $(1) := $(2)
	endif
	ifeq ($(origin $1),undefined)
		override $(1) := $(2)
	endif
endef
 
# It is highly recommended to use a custom built cross toolchain to build a kernel.
# We are only using "cc" as a placeholder here. It may work by using
# the host system's toolchain, but this is not guaranteed.
override DEFAULT_CC := x86_64-elf-gcc
$(eval $(call DEFAULT_VAR,CC,$(DEFAULT_CC)))
 
# Same thing for "ld" (the linker).
override DEFAULT_LD := x86_64-elf-ld
$(eval $(call DEFAULT_VAR,LD,$(DEFAULT_LD)))
 
# User controllable C flags.
override DEFAULT_CFLAGS := -g -O2 -pipe
$(eval $(call DEFAULT_VAR,CFLAGS,$(DEFAULT_CFLAGS)))
 
# User controllable C preprocessor flags. We set none by default.
override DEFAULT_CPPFLAGS :=
$(eval $(call DEFAULT_VAR,CPPFLAGS,$(DEFAULT_CPPFLAGS)))
 
# User controllable linker flags. We set none by default.
override DEFAULT_LDFLAGS :=
$(eval $(call DEFAULT_VAR,LDFLAGS,$(DEFAULT_LDFLAGS)))
 
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
	-mabi=sysv \
	-mno-80387 \
	-mno-mmx \
	-mno-sse \
	-mno-sse2 \
	-mno-red-zone \
	-mcmodel=kernel \
	-DSYS_SELF_TEST

# TODO: add these flags: -Wconversion -Wsign-conversion -Wcast-align \
# -Wpointer-arith -Wshadow
 
# Internal C preprocessor flags that should not be changed by the user.
override CPPFLAGS := \
	-I include \
	-I. \
	$(CPPFLAGS) \
	-MMD \
	-MP \
	-DSYS_SELF_TEST
 
# Internal linker flags that should not be changed by the user.
override LDFLAGS += \
	-nostdlib \
	-static \
	-m elf_x86_64 \
	-z max-page-size=0x1000 \
	-T src/linker.ld
 
# Check if the linker supports -no-pie and enable it if it does.
ifeq ($(shell $(LD) --help 2>&1 | grep 'no-pie' >/dev/null 2>&1; echo $$?),0)
	override LDFLAGS += -no-pie
endif
 
# Use "find" to glob all *.c, *.s, and *.asm files in the tree and obtain the
# object and header dependency file names.
override CFILES := $(shell find -L . -type f -name '*.c')
override ASFILES := $(shell find -L . -type f -name '*.s')

# Output directory for the build. Can be overridden.
override BUILD_DIR := ../build/kernel

override OBJ := $(patsubst %.c,$(BUILD_DIR)/%.c.o,$(CFILES)) $(patsubst %.s,$(BUILD_DIR)/%.s.o,$(ASFILES))
override HEADER_DEPS := $(patsubst %.c,$(BUILD_DIR)/%.c.d,$(CFILES)) $(patsubst %.s,$(BUILD_DIR)/%.s.d,$(ASFILES))
 
# Default target.
.PHONY: all
all: $(BUILD_DIR)/$(KERNEL)
 
# Link rules for the final kernel executable.
$(BUILD_DIR)/$(KERNEL): $(OBJ)
	$(LD) $(OBJ) $(LDFLAGS) -o $@
 
# Include header dependencies.
-include $(HEADER_DEPS)
 
# Compilation rules for *.c files.
$(BUILD_DIR)/%.c.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@
 
# Compilation rules for *.S files.
$(BUILD_DIR)/%.s.o: %.s
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@
 
# Remove object files and the final executable.
.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)

# Format C source and header files (only touches src/ and main/)
.PHONY: format
format:
	find src/ include/ -type f \( -name '*.c' -o -name '*.h' \) ! -name 'limine.h' -exec clang-format -i {} +
