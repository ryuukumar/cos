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

heading "Removing all build files" "1;33"

rm -rvf build

heading "Removing ISO image" "1;33"

[ -f image.iso ] && rm -v image.iso

heading "Cleaning complete" "1;32"
