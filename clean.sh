#!/bin/sh

printf "\e[1;33m\n# ---+\n# Removing all build files\n# ---+\n\e[0m\n"

rm -rf build
rm -rf limine

printf "\e[1;33m\n# ---+\n# Removing ISO image\n# ---+\n\e[0m\n"

rm -v image.iso

printf "\e[1;32m\n# ---+\n# Cleaning complete\n# ---+\n\e[0m\n"
