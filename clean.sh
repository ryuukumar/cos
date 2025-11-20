
echo -e "\e[1;33m\n# ---+\n# Removing all build files\n# ---+\n\e[0m"

rm -rf build
rm -rf limine

echo -e "\e[1;33m\n# ---+\n# Removing ISO image\n# ---+\n\e[0m"

rm -v image.iso

echo -e "\e[1;32m\n# ---+\n# Cleaning complete\n# ---+\n\e[0m"
