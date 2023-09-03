# todds

[![License: MPL 2.0](https://img.shields.io/badge/License-MPL%202.0-brightgreen.svg)](https://opensource.org/licenses/MPL-2.0) [![Contributor Covenant](https://img.shields.io/badge/Contributor%20Covenant-2.1-4baaaa.svg)](CODE_OF_CONDUCT.md)

A CPU-based DDS encoder optimized for fast batch conversions with high encoding quality.

## Features

```
ARGS:
  input                       Encode all PNG files inside of this folder as DDS. It can also point to a single PNG file. If this parameter points to a TXT file, it will be processed as a list of PNG files and/or directories. Entries must be on separate lines. Every listed PNG file and those inside listed directories will be encoded as DDS.
  output                      Write DDS files to this folder instead of creating them next to input PNGs. This argument is ignored if input points to a TXT file.

OPTIONS:
  -cl, --clean                Deletes all DDS files matching input PNG files instead of encoding them.
  -f, --format                DDS encoding format.
                                  BC7: High-quality compression supporting alpha. [Default]
                                  BC1: Highly compressed RGB data.
  -af, --alpha-format         Use a different DDS encoding format for files with alpha. Defaults to using the value in --format unconditionally.
                                  BC7: High-quality compression supporting alpha. [Default]
  -q, --quality               Encoder quality level, must be in [0, 7]. Defaults to 6.
  -nm, --no-mipmaps           Disable mipmap generation.
  -fs, --fix-size             Set image width and height to the next multiple of 4.
  -mf, --mipmap-filter        Filter used to resize images during mipmap generation.
                                  LANCZOS: Lanczos interpolation. Preserves edges and details better than other filters when dowsncaling images. [Default]
                                  NEAREST: Nearest neighbor interpolation. Very fast, but it does not produce great results.
                                  LINEAR: Bilinear interpolation. Fast, and with reasonable quality.
                                  CUBIC: Bicubic interpolation. Recommended filter for upscaling images.
                                  AREA: Resampling using pixel area relation. Good for downscaling images and mipmap generation.
  -mb, --mipmap-blur          Blur applied during mipmap generation. Defaults to 0.55.
  -sc, --scale                Scale image size by a value given in %.
  -ms, --max-size             Downscale images with a width or height larger than this threshold to fit into it.
  -sf, --scale-filter         Filter used to scale images when using the scale or max_size parameters.
                                  LANCZOS: Lanczos interpolation. Preserves edges and details better than other filters when dowsncaling images. [Default]
                                  NEAREST: Nearest neighbor interpolation. Very fast, but it does not produce great results.
                                  LINEAR: Bilinear interpolation. Fast, and with reasonable quality.
                                  CUBIC: Bicubic interpolation. Recommended filter for upscaling images.
                                  AREA: Resampling using pixel area relation. Good for downscaling images and mipmap generation.
  -th, --threads              Number of threads used by the parallel pipeline, must be in [1, 32]. Defaults to maximum.
  -d, --depth                 Maximum subdirectory depth to use when looking for source files. Defaults to maximum.
  -o, --overwrite             Convert files even if an output file already exists.
  -on, --overwrite-new        Convert files if an output file exists, but it is older than the input file.
  -vf, --vflip                Flip source images vertically before encoding.
  -t, --time                  Show total execution time.
  -r, --regex                 Process only absolute paths matching this regular expression.
  -dr, --dry-run              Calculate all files that would be affected but do not make any changes.
  -p, --progress              Display progress messages.
  -v, --verbose               Display all input files of the current operation.
  -h, --help                  Show usage information.

ADVANCED OPTIONS:
  -bc1-ab, --bc1-alpha-black  The BC1 encoder will use 3 color blocks for blocks containing black or very dark pixels. Increases texture quality substantially, but programs using these textures must ignore the alpha channel.
  -rp, --report               Prints information about the encoding process of each file.
```

### Quality

BC7 support is implemented using the [bc7e.ispc](https://github.com/richgel999/bc7enc_rdo) library. BC1 support has been implemented with the [rgbcx](https://github.com/richgel999/bc7enc_rdo) library.

As seen in the [Texture Compression in 2020](https://aras-p.info/blog/2020/12/08/Texture-Compression-in-2020/) blog post, these libraries provide great encoding quality and performance.

Check the [Analysis documentation](ANALYSIS.md) for details.

### Performance

todds is optimized for encoding multiple files at the same time, without compromising on encoding quality. The full benefits of todds are reached when it is used to encode large numbers of files, but it also performs well while encoding single files.

Check the [Analysis documentation](ANALYSIS.md) for details.

## Building

Compiling todds requires a recent C++ compiler version, the [Intel® Implicit SPMD Program Compiler](https://github.com/ispc/ispc) and [CMake](https://cmake.org/).

### Dependencies

To compile todds, the following dependencies must be available as development libraries. All dependencies are available through [vcpkg](https://github.com/microsoft/vcpkg).

* [Boost.Filesystem](https://www.boost.org/doc/libs/master/libs/filesystem/doc/index.htm)
* [Boost.NoWide](https://www.boost.org/doc/libs/master/libs/nowide/doc/html/index.html)
* [Boost.String](https://www.boost.org/doc/libs/master/doc/html/string_algo.html)
* [Catch2](https://github.com/catchorg/Catch2): Only required if `TODDS_UNIT_TESTS` is set to on.
* [Hyperscan](https://www.hyperscan.io): Required for regular expression support. Only required when `TODDS_REGULAR_EXPRESSIONS` is set to `ON`.
* [fmt](https://fmt.dev/latest/index.html)
* [oneTBB](https://github.com/oneapi-src/oneTBB)
* [OpenCV](https://opencv.org/)

The following third party library dependencies are contained as source code in the thirdparty folder of the todds repository. Each one of these libraries is under its own license which is also included in the repository.

* [bc7enc_rdo](https://github.com/richgel999/bc7enc_rdo)
* [libspng](https://libspng.org/)
* [miniz](https://github.com/richgel999/miniz)

### CMake options

* `TODDS_CLANG_ALL_WARNINGS`: This option is only available when the clang compiler is in use. This enables almost every Clang warning, except for a few that cause issues with todds. This may trigger unexpected positives when using newer Clang versions. Off by default.
* `TODDS_CLANG_TIDY`: If [clang-tidy](https://clang.llvm.org/extra/clang-tidy/) is available, it will be used to analyze the project. Off by default.
* `TODDS_MIMALLOC_ALLOCATOR`: todds will use the [mimalloc](https://github.com/microsoft/mimalloc) allocator instead of the standard allocator.
* `TODDS_NEON_SIMD`: Use NEON SIMD instructions instead of x64 SIMD instructions. Intended for compiling for ARM platforms.
* `TODDS_PIPELINE_DUMP`: Dump the binary data generated by each pipeline stage. Limited to a single file. The dump files will be generated next to the todds executable.
* `TODDS_REGULAR_EXPRESSIONS`: Enables the regular expression parameter for todds. Requires the [Hyperscan](https://github.com/intel/hyperscan) library.
* `TODDS_TBB_ALLOCATOR` todds will use the [scalable_allocator](https://oneapi-src.github.io/oneTBB/main/tbb_userguide/Memory_Allocation.html) from [oneTBB](https://github.com/oneapi-src/oneTBB) instead of the standard allocator.
* `TODDS_UNIT_TESTS` -> Build todds unit tests. Requires the [Catch2](https://github.com/catchorg/Catch2) library. Off by default.
* `TODDS_WARNINGS_AS_ERRORS`: Treat all compiler warnings as errors. Off by default.
* `TODDS_TRACY`: Compiles todds with [Tracy Profiler](https://github.com/wolfpld/tracy) support. todds will use a custom allocator that will expose allocation data to Tracy.

## Contributing

todds encourages community involvement and contributions. Check the [CONTRIBUTING.md](CONTRIBUTING.md) and [CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md) files for details. You can check all contributors in the [contributors list](https://github.com/joseasoler/todds/graphs/contributors).

You should enable the `TODDS_UNIT_TESTS`, `TODDS_WARNINGS_AS_ERRORS` and `TODDS_CLANG_TIDY` CMake options to ensure that your contribution will pass static analysis.

## License

todds is licensed under the Mozilla Public License, v. 2.0. See the [LICENSE](LICENSE) file for details. Check the [MPL 2.0 FAQ](https://www.mozilla.org/en-US/MPL/2.0/FAQ/) to learn more.

## Acknowledgements

Read the [ACKNOWLEDGEMENTS.md](ACKNOWLEDGEMENTS.md) file for details.
