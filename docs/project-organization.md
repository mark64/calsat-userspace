# Project Organization

## Subdirectories

Each separate process should have its own subdirectory. For example, all source
files related to the telemetry daemon are located in `telem`. Similarly, sources
for the orientation daemon (which controls detumbling and pointing) are located
in `orienation`. This design keeps the repo organized and helps everyone find
code, so please follow it.

In addition, there are some special subdirectories listed below.

### build*

Any subdirectory beginning with `build`, such as `build_native`, contain
compiler output such as libraries and processes. These directories are generated
by the `Makefile` and populated by `cmake`.

### docs

Documentation related to the software for the CubeSat, including software from
this repo and others such as
[calsat-kernel-modules](https://github.com/space-technologies-at-california/calsat-kernel-modules).

### include

This subdirectory holds all the header files for this repo. Every process
subdirectory (`telem`, `orientation`, etc.) has a similarly named subdirectory
here, except only header files are stored here. Please keep all header files
here, rather than in your top level subdirectory, because it makes it easier for
people to understand the system architecture.

_**All header files should be extremely well documented. I will reject pull
requests if I can't understand your code after reading your header files and/or
docs**_.

### scripts

All shell and python scripts related to the build process, toolchains, testing,
or other actions not part of the main CubeSat software go here.

See [build-process](build-process,md) for further explanation of toolchain
scripts.
