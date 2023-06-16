
function heading() {
  input_string=$1
  string_length=${#input_string}
  line_char="-"

  # Output hashtags
  final_string=""
  for ((i=1; i<=string_length+4; i++))
  do
    final_string+=$line_char
  done
  final_string+="\n"  # Add a newline after the hashtags
  
  final_string+="# ${input_string} #\n"
  
  # Output hashtags
  for ((i=1; i<=string_length+4; i++))
  do
    final_string+=$line_char
  done
  final_string+="\n"  # Add a newline after the hashtags

  echo -e "$final_string"
}

echo -e "\e[1;33m\n$(heading "Checking for compilers")\n\e[0m"

if ! command -v x86_64-elf-gcc &> /dev/null; then
	echo -e "\e[1;31m\n$(heading "ERROR: x86_64-elf-gcc not found.")\n\e[0m"
	echo -e "You probably don't have a cross-compiler installed, or it is in the wrong path. Please refer online on how you can build and install one."
	exit
else
	echo -e "x86_64-elf-gcc found"
fi

if ! command -v x86_64-elf-g++ &> /dev/null; then
	echo -e "\e[1;31m\n$(heading "ERROR: x86_64-elf-g++ not found.")\n\e[0m"
	echo -e "You probably don't have a cross-compiler installed, or it is in the wrong path. Please refer online on how you can build and install one."
	exit
else
	echo -e "x86_64-elf-g++ found"
fi

if ! command -v x86_64-elf-as &> /dev/null; then
	echo -e "\e[1;31m\n$(heading "ERROR: x86_64-elf-as not found.")\n\e[0m"
	echo -e "You probably don't have a cross-compiler installed, or it is in the wrong path. Please refer online on how you can build and install one."
	exit
else
	echo -e "x86_64-elf-as found"
fi

if ! command -v nasm &> /dev/null; then
	echo -e "\e[1;31m\n$(heading "ERROR: nasm not found.")\n\e[0m"
	echo -e "nasm can be simply installed with: sudo apt install nasm"
	exit
else
	echo -e "nasm found"
fi

echo -e "\e[1;33m\n$(heading "Building OS binaries")\n\e[0m"

make -j4

echo -e "\nentry.elf generated with size $(wc -c <"build/entry.elf") bytes"

echo -e "\e[1;33m\n$(heading "Building Limine-deploy")\n\e[0m"

# Download the latest Limine binary release.
git clone https://github.com/limine-bootloader/limine.git --branch=v4.x-branch-binary --depth=1
 
# Build limine-deploy.
make -C limine

echo -e "\e[1;33m\n$(heading "Populating ISO directory")\n\e[0m"

# Create a directory which will be our ISO root.
mkdir -p iso_root
 
# Copy the relevant files over.
cp -v build/entry.elf limine.cfg limine/limine.sys \
      limine/limine-cd.bin limine/limine-cd-efi.bin iso_root/
 
# Create the EFI boot tree and copy Limine's EFI executables over.
mkdir -p iso_root/EFI/BOOT
cp -v limine/BOOT*.EFI iso_root/EFI/BOOT/
 
echo -e "\e[1;33m\n$(heading "Creating ISO")\n\e[0m"
 
# Create the bootable ISO.
xorriso -as mkisofs -b limine-cd.bin \
        -no-emul-boot -boot-load-size 4 -boot-info-table \
        --efi-boot limine-cd-efi.bin \
        -efi-boot-part --efi-boot-image --protective-msdos-label \
        iso_root -o image.iso \
        && rm -rvf iso_root
 
# Install Limine stage 1 and 2 for legacy BIOS boot.
./limine/limine-deploy image.iso

echo -e "\nimage.iso generated with size $(wc -c <"image.iso") bytes"

echo -e "\e[1;32m\n$(heading "ISO generated successfully")\n\e[0m"
