
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

echo -e "\e[1;33m\n$(heading "Running QEMU")\n\e[0m"

echo "Running image.iso as CD-ROM image."
qemu-system-x86_64 -cdrom image.iso -vga std

echo -e "\e[1;32m\n$(heading "Run complete")\n\e[0m"
