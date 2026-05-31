# COS - A hobby OS project

COS is a hobby OS development project which I sporadically work on to further my programming skills in C and C++ and my understanding of computer systems. I intend to stick to these two languages for the whole project.

Documentation is sparse, but you can use doxygen to build it from the in-code comments.

**DISCLAIMER:** COS is not Windows or Linux or what-have-it, I cannot vouch for its stability! Though I make my best effort to ensure it works as expected, running this on your potato may end up cooking it! **Please don't run this on any even remotely important hardware.** VMs are okay :)

## Capabilities

- It boots up on most modern-day 64-bit systems that support x86_64 (so no ARM)
- It can run a simple bash-like shell that allows you to execute ELF executables and perform some elementary scripting
- It can host C applications that can compile with newlib as libc, and exposes 26 system calls for file operations, memory management, process management, etc.
- More to come

## Setup

### Prerequisites

In order from `critical` to `meh`:

- [required] `x86_64-rcos` targeting cross compiler
- [required] `git` so we can download limine (`sudo apt install git` or similar)
- [required] `xorriso` to build ISOs (`sudo apt install xorriso` or similar)
- [optional] `doxygen` for documentation (`sudo apt install doxygen` or similar)
- [optional] `qemu-system-x86` if you want to use `qemu.sh` to run COS (`sudo apt install qemu-system-x86` or similar)

#### Setting up cross-compiling toolchain

You can run the build script `tools/build_toolchain`. It should automatically download the required gcc and binutils, configure these and newlib, and build an x86_64-rcos targeting toolchain. Run it from within the tools directory:

```sh
cd tools && ./build_toolchain
```

**Note that building a GCC toolchain can take upwards of an hour on some systems.** Please set aside some time for this task.

By default this sets up GCC 15.2.0 and Binutils 2.45 in `cos/toolchain`. You can add this location to your path, or move the built toolchain to a more convenient location. You can also modify INSTALL_DIR in the build script to your liking.

It is strongly recommended not to change the versions of GCC and Binutils to be built, as there are no guarantees that any two pairs of versions will result in working software; additionally, the patch files will likely not work on any other version, and will require changes on your part.

The build script has been tested and found working on Linux (Arch) kernel 7.0.10 and macOS 26. Do note that you may need to install via preferred package manager some prerequisites as applicable, such as `gcc` or `build-essentials`.

### Actually building (and running) COS

You can run the `build.sh` script to build COS (assuming the prerequisites are available):

```sh
./build.sh
```

This will generate a file `image.iso` inside your working directory. Use VMWare or your favourite VM emulator (or put it on a liveCD if you're brave enough) to run and test it out. Or run `qemu.sh` to run it on QEMU.

![An image of COS saying 'Hello, world!'](images/hello%20world%20with%20cosh.png)

## License

This project is released under the GPLv2 license. To find out more, read [LICENSE](LICENSE).
