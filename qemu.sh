#!/bin/sh

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

heading "Running QEMU" "1;33"

echo "Running image.iso as CD-ROM image."
qemu-system-x86_64 -cdrom image.iso -vga std -machine accel=tcg -serial file:serial.log -s -S

heading "Run complete" "1;32"
