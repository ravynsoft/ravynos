# Building ravynOS From Source

This guide is for the current Darwin-based ravynOS. A new CMake-based build system is being created for portability and maintainability. Structurally, the build starts by creating a toolchain for the host OS and then using it to build the OS for the same CPU architecture.

The host toolchain (Default.xctoolchain) includes clang, LLVM, Apple cctools, xcbuild, and other utilities needed to build the OS. Building LLVM initially takes a __long__ time, so be patient. It lives in `<build_dir>/Developer/Toolchains/Default.xctoolchain` once built.

After the toolchain is available, critical system libraries and headers will be built into the ravynOS.sdk - an equivalent of MacOSX.sdk for ravynOS native targets. The new SDK is then used to progressively build the rest of the system.

Building it this way takes longer, but there are advantages. It helps make the entire build portable and it guarantees build consistency, which helps reduce those nasty "well it works for me" problems. Once it has built fully, subsequent incremental builds are faster.

## What you need:
* A host machine with:
  * clang 17.x (Apple, Linux) or 18.x (Linux)
  * cmake 3.15+ and ninja
  * GNU make (gmake)
  * BSD make (bmake)
  * Python 3
  * OpenSSL
  * libpng (for xcbuild), libxml2, zlib (libz)
* A lot of patience


__On macOS:__
* Have a recent Xcode installed
* Run: `cmake -S /path/to/ravynos -B /path/to/build -GNinja`
* Run: `cmake --build /path/to/build`


__On Linux (tested on Arch 2025/12/21):__
* Install `csh`, `bison`, `binutils`, `flex`, `libdispatch`, and the LLVM linker `lld`
* Symlink `cc` and `c++` to clang to avoid accidental use of gcc: `rm -f /usr/bin/cc /usr/bin/c++; ln -sf clang /usr/bin/cc; ln -sf clang++ /usr/bin/c++`
* Run: `cmake --fresh -S /path/to/ravynos -B /path/to/build -GNinja`
* Run: `cmake --build /path/to/build`
