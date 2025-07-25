# COS - A hobby OS project

COS is a hobby OS development project which I sporadically work on to further my programming skills in C and C++ and my understanding of computer systems. I intend to stick to these two languages for the whole project.

Documentation is sparse, but you can use doxygen to build it from the in-code comments.

**DISCLAIMER:** COS is not Windows or Linux or what-have-it, I cannot vouch for its stability! Though I make my best effort to ensure it works as expected, running this on your potato may end up cooking it! **Please don't run this on any even remotely important hardware.** VMs are okay :)

## Capabilities

- It boots up on most modern-day 64-bit systems that support x86_64 (so no ARM)
- It can print stuff on your screen
- It can do some basic memory management with paging
- More to come

## Setup

### Prerequisites

In order from `critical` to `meh`:

- [required] `x86_64-elf` targeting cross compiler ([I wonder how you could set that up](https://github.com/ryuukumar/cross-compiler-compiler))
- [required] `git` so we can download limine (`sudo apt install git` or similar)
- [required] `xorriso` to build ISOs (`sudo apt install xorriso` or similar)
- [optional] `doxygen` for documentation (`sudo apt install doxygen` or similar)
- [optional] `qemu-system-x86` if you want to use `qemu.sh` to run COS (`sudo apt install qemu-system-x86` or similar)

### Actually building (and running) COS

You can run the following code to download and build COS:

```
git clone https://github.com/ryuukumar/cos.git && cd cos
chmod +x build.sh
./build.sh
```

This will generate a file `image.iso` inside your working directory. Use VMWare or your favourite VM emulator (or put it on a liveCD if you're brave enough) to run and test it out. Or run `qemu.sh` to run it on QEMU.

![An image of COS saying 'Hello, world!'](images/hello%20world.png)
