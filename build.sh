#!/bin/sh

function heading() {
	input_string=$1
	string_length=${#input_string}
	line_char="-"

    # Generate line
    line=""
    i=0
    while [ $i -lt $((string_length + 4)) ]; do
        line="${line}${line_char}"
        i=$((i + 1))
    done

	printf "\n%s\n# %s #\n%s\n\n" "$line" "$input_string" "$line"
}

function show_help() {
	printf "\e[1;32m$(heading "Build Script Help")\e[0m\n"
	printf "Usage: ./build.sh [OPTIONS]\n\n"
	printf "Options:\n"
	printf "  -d, --docs     Build documentation with Doxygen\n"
	printf "  -f, --fresh    Clean build directory before building\n"
	printf "  -h, --help     Show this help message and exit\n"
}

function clean_build() {
	printf "\e[1;33m\n$(heading "Removing all build files")\n\e[0m\n"
	rm -rf build
	rm -rf limine

	printf "\e[1;33m\n$(heading "Removing ISO image")\n\e[0m\n"
	rm -v image.iso

	printf "\e[1;32m\n$(heading "Cleaning complete")\n\e[0m\n"
}

# Parse command line arguments
BUILD_DOCS=false
FRESH_BUILD=false

while [[ $# -gt 0 ]]; do
	case $1 in
		-d|--docs)
			BUILD_DOCS=true
			shift
			;;
		-f|--fresh)
			FRESH_BUILD=true
			shift
			;;
		-h|--help)
			show_help
			exit 0
			;;
		*)
			echo "Unknown option: $1"
			echo "Use --help for usage information."
			exit 1
			;;
	esac
done

# Clean build if requested
if [ "$FRESH_BUILD" = true ]; then
	clean_build
fi

printf "\e[1;33m\n$(heading "Checking for compilers")\n\e[0m\n"

if ! command -v x86_64-elf-gcc &> /dev/null; then
	printf "\e[1;31m\n$(heading "ERROR: x86_64-elf-gcc not found.")\n\e[0m\n"
	printf "You probably don't have a cross-compiler installed, or it is in the wrong path. Please refer online on how you can build and install one.\n"
	exit
else
	printf "x86_64-elf-gcc found"
fi

if ! command -v x86_64-elf-g++ &> /dev/null; then
	printf "\e[1;31m\n$(heading "ERROR: x86_64-elf-g++ not found.")\n\e[0m\n"
	printf "You probably don't have a cross-compiler installed, or it is in the wrong path. Please refer online on how you can build and install one.\n"
	exit
else
	printf "x86_64-elf-g++ found"
fi

if ! command -v x86_64-elf-as &> /dev/null; then
	printf "\e[1;31m\n$(heading "ERROR: x86_64-elf-as not found.")\n\e[0m\n"
	printf "You probably don't have a cross-compiler installed, or it is in the wrong path. Please refer online on how you can build and install one.\n"
	exit
else
	printf "x86_64-elf-as found"
fi

if [ "$BUILD_DOCS" = true ]; then
	printf "\e[1;33m\n$(heading "Building documentation")\n\e[0m\n"
	doxygen Doxyfile
fi

printf "\e[1;33m\n$(heading "Building OS binaries")\n\e[0m\n"

make -j$(nproc)

printf "\nentry.elf generated with size $(wc -c <"build/kernel/entry.elf") bytes\n"

printf "\e[1;33m\n$(heading "Building Limine-deploy")\n\e[0m\n"

# Download the latest Limine binary release.
git clone https://github.com/limine-bootloader/limine.git --branch=v9.x-binary --depth=1
 
# Build limine-deploy.
make -C limine

printf "\e[1;33m\n$(heading "Populating ISO directory")\n\e[0m\n"

# Create a directory which will be our ISO root.
mkdir -p iso_root/boot/limine
 
# Copy the relevant files over.
cp -v limine.conf limine/limine-bios.sys limine/limine-bios-cd.bin limine/limine-uefi-cd.bin iso_root/boot/limine/
cp -v build/kernel/entry.elf iso_root/boot/
cp -v build/initramfs.cpio iso_root/boot/
 
# Create the EFI boot tree and copy Limine's EFI executables over.
mkdir -p iso_root/EFI/BOOT
cp -v limine/BOOT*.EFI iso_root/EFI/BOOT/
 
printf "\e[1;33m\n$(heading "Creating ISO")\n\e[0m\n"
 
# Create the bootable ISO.
xorriso -as mkisofs -b boot/limine/limine-bios-cd.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table \
		--efi-boot boot/limine/limine-uefi-cd.bin \
		-efi-boot-part --efi-boot-image --protective-msdos-label \
		iso_root -o image.iso \
		&& rm -rvf iso_root
 
# Install Limine stage 1 and 2 for legacy BIOS boot.
#./limine/limine-deploy image.iso
./limine/limine bios-install image.iso

printf "\nimage.iso generated with size $(wc -c <"image.iso") bytes\n"

printf "\e[1;32m\n$(heading "ISO generated successfully")\n\e[0m\n"
