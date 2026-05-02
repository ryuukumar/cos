#!/bin/sh

NPROC=$(getconf _NPROCESSORS_ONLN)
ROOTDIR=$(pwd)

export MAKE=make
if [ "$(uname)" = "Darwin" ]; then
    if command -v gmake >/dev/null 2>&1; then
        export MAKE=gmake # build could fail if using BSD make. use gmake if possible
    else
        printf "You are on macOS but gmake was not found. I will use make but build could fail.\nRetry with GNU make (e.g. brew install make) if build fails.\n" >&2
    fi
fi

function heading() {
	input_string="$1"
	output_color="$2"
	line_char="-"

	len=${#input_string}
	width=$((len + 4))
	line=$(printf '%*s' "$width" '' | tr ' ' "$line_char")

	if [ -n "$output_color" ]; then
		printf '\e[%sm\n%s\n%s\n%s\n\n\e[0m' "$output_color" "$line" "# ${input_string} #" "$line"
	else
		printf '\n%s\n%s\n%s\n\n' "$line" "# ${input_string} #" "$line"
	fi
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
	heading "Removing all build files" "1;33"
	rm -rvf build
	rm -rvf limine

	heading "Removing ISO image" "1;33"
	[ -f image.iso ] && rm -v image.iso

	heading "Cleaning complete" "1;32"
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

commands="x86_64-elf-gcc x86_64-elf-g++ x86_64-elf-ld x86_64-elf-as"
for cmd in $commands; do
	if ! command -v "$cmd" &> /dev/null; then
		heading "ERROR: $cmd not found." "1;31"
		printf "You probably don't have a cross-compiler installed, or it is in the wrong path. Please refer online on how you can build and install one.\n"
		exit
	else
		printf "$cmd found\n"
	fi
done

if [ "$BUILD_DOCS" = true ]; then
	heading "Building documentation" "1;33"
	doxygen Doxyfile
fi

heading "Applying patches" "1;33"

if git -C "$ROOTDIR/lib/newlib-cygwin" apply --check --reverse "$ROOTDIR/patches/newlib/0001-cos-syscall-stubs.patch" 2>/dev/null; then
	printf "Already applied: $ROOTDIR/patches/newlib/0001-cos-syscall-stubs.patch\n"
else
	git -C "$ROOTDIR/lib/newlib-cygwin" apply "$ROOTDIR/patches/newlib/0001-cos-syscall-stubs.patch"
	printf "Applied: $ROOTDIR/patches/newlib/0001-cos-syscall-stubs.patch\n"
fi

heading "Configuring libc" "1;33"

mkdir -p build/lib/newlib
cd build/lib/newlib
[ -f Makefile ] || ../../../lib/newlib-cygwin/configure --target=x86_64-elf --prefix=/usr --disable-multilib

heading "Building libc" "1;33"

$MAKE all -j$NPROC
$MAKE DESTDIR=$(pwd)/../../../initramfs tooldir=/usr install

cd "$ROOTDIR"

export CC=x86_64-elf-gcc
export CXX=x86_64-elf-g++
export LD=x86_64-elf-ld
export AS=x86_64-elf-as
export AR=x86_64-elf-ar

$MAKE -j$NPROC lib

heading "Building OS binaries" "1;33"

$MAKE -j$NPROC kernel

if [ $? -ne 0 ]; then
    printf "\n$MAKE exited with errors. Not proceeding with build.\n"
	exit
fi

printf "\nentry.elf generated with size $(wc -c <"build/kernel/entry.elf") bytes\n"

heading "Building userspace" "1;33"

$MAKE -j$NPROC initramfs

if [ $? -ne 0 ]; then
    printf "\n$MAKE exited with errors. Not proceeding with build.\n"
	exit
fi

printf "\ninitramfs.cpio generated with size $(wc -c <"build/initramfs.cpio") bytes\n"

heading "Building Limine-deploy" "1;33"

$MAKE -C limine

if [ $? -ne 0 ]; then
    printf "\n$MAKE for limine exited with errors. Not proceeding with build.\n"
	exit
fi

heading "Populating ISO directory" "1;33"

# Create a directory which will be our ISO root.
mkdir -p iso_root/boot/limine
 
# Copy the relevant files over.
cp -v limine.conf limine/limine-bios.sys limine/limine-bios-cd.bin limine/limine-uefi-cd.bin iso_root/boot/limine/
cp -v build/kernel/entry.elf iso_root/boot/
cp -v build/initramfs.cpio iso_root/boot/
 
# Create the EFI boot tree and copy Limine's EFI executables over.
mkdir -p iso_root/EFI/BOOT
cp -v limine/BOOT*.EFI iso_root/EFI/BOOT/

heading "Creating ISO" "1;33"
 
# Create the bootable ISO.
xorriso -as mkisofs -b boot/limine/limine-bios-cd.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table \
		--efi-boot boot/limine/limine-uefi-cd.bin \
		-efi-boot-part --efi-boot-image --protective-msdos-label \
		iso_root -o image.iso \
		&& rm -rvf iso_root
 
# Install Limine stage 1 and 2 for legacy BIOS boot.
./limine/limine bios-install image.iso

printf "\nimage.iso generated with size $(wc -c <"image.iso") bytes\n"

heading "ISO generated successfully" "1;32"
