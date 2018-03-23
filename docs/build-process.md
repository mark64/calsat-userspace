# Build Process

## Configuration Setup

While this project is designed to be run on an ARM processor with a host of
sensors and special hardware, the build supports multiple targets, including the
host system's CPU, Raspberry Pi 0/1/2, and the Digi ConnectCore 6UL (the one we
use for our CubeSat).

You can view all available configs by running `make listconfigs`. To build this
project for the host CPU (useful for running tests and other code that doesn't
depend on hardware), you first have to run `make native-config`. This will
generate a `.config` file that is sourced by the `Makefile` in order to
determine the toolchain (compiler, linker, standard libraries) to use.

## Compiling

After you have selected a configuration, to build the project run `make`. The
toolchain setup script should have already installed the toolchain and prepared
`cmake` to use the proper compiler.

## Troubleshooting Build Issues

Because of the way `cmake` and the default `CMakeLists.txt` for each
subdirectory is setup, if you add a new file `cmake` will not detect it. To fix
this, run `make depends`. Also, consider manually specifying the `SOURCES` in
your `CMakeLists.txt`.

There are multiple `clean` targets available to suit your level of paranoia:
`make clean`, which runs `cmake`'s default `clean` target; `make distclean`,
which removes most generated files for your selected board; and `make
reallyclean`, which removes every generated file. I personally find that none of
the `clean` targets fix my issues, but it's nice to be able to remove the
existing object files from your list of suspects.

## Config Generation

Available configs are generated automagically by the `Makefile` by reading the
filenames with the prefix `install-toolchain-` in the `scripts/` subdirectory.
When it finds a file named, for example, `install-toolchain-rpi0`, it creates a
target called `rpi0-config`. When you run `make rpi0-config`, `Makefile` runs
the `install-toolchain-rpi0.sh` script and creates a `.config` file to indicate
the current toolchain. All builds will then be placed in the `build_rpi0/`
subdirectory.

If you want to add a new board to the set of supported boards, make a new
`install-toolchain-<board>.sh` script and place it in the `scripts`
subdirectory. Take a look at the rpi0 script to see what needs to be done.
