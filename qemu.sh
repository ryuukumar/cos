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

	echo -e "\n$line\n# $input_string #\n$line\n\n"
}

echo -e "\e[1;33m\n$(heading "Running QEMU")\n\e[0m"

echo "Running image.iso as CD-ROM image."
qemu-system-x86_64 -cdrom image.iso -vga std -serial file:serial.log

echo -e "\e[1;32m\n$(heading "Run complete")\n\e[0m"
