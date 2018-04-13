# Build Process

## Toolchain Setup

While this project is designed to be run on an ARM processor with a host of
sensors and special hardware, the build supports multiple targets, including the
host system's CPU, Raspberry Pi 0/1/2, and the Digi ConnectCore 6UL (the one we
use for our CubeSat).

You can view all available toolchain files by running

```sh
$ make listconfigs
```

To select a cross compilation toolchain, run the following inside your build
folder:

```sh
$ cmake ../ -DCMAKE_TOOLCHAIN_FILE=<toolchain_file.cmake>
```

Note that the packaged cross compilation toolchains assume your host machine is
x64_86. To build this project for the host CPU, omit the
`-DCMAKE_TOOLCHAIN_FILE` flag.

## Compiling

After you have run `cmake`, to build the project run:

```sh
make
```

## Troubleshooting Build Issues

Because of the way `cmake` and the default `CMakeLists.txt` for each
subdirectory is setup, if you add a new file `cmake` will not detect it. To fix
this, run `cmake ...` again. Also, consider manually specifying the `SOURCES` in
your `CMakeLists.txt` to avoid this.

Sometimes the existing object files or configuration files can cause issues with
builds. A good first step is to run:

```sh
make clean
```

If that doesn't work and you're sure your code is correct, you can try removing
the build folder and starting over. If neither of those fix the issue, check all
your `CMakeLists.txt` and source code files.

## Toolchain Files

The existing cmake toolchain files are located in
[cmake/toolchains](cmake/toolchains).

If you want to add a new board to the set of supported boards, make a new
`<toolchain_name>.cmake` file and place it in the subdirectory noted above. Take
a look at the existing files to see what needs to be done to add a cross
compilation toolchain.
