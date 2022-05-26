# Ubuntu Build Instructions (Only tested on 18.04 LTS)

1. Intall the required packages from the provided file:
```
$ sudo apt install $(cat ubuntu_apt_requirements.txt)
```

2. Build GTest:
The Ubuntu package for GTest only installs the headers and source &#x1F644;:
```
$ mkdir gtest_build
$ cd gtest_build/
$ cmake /usr/src/googletest/googletest
$ make
$ sudo cp lib/libgtest* /usr/lib/
```

2. Install the dependencies that are not available via apt. These are currently:
- Lua 5.2 https://www.lua.org/
- GLK https://github.com/mpups/glk

3. Build using scons:
```
$ scons
```

4. If anything fails your version of Ubuntu might install the dependencies
in differing locations. Check the location for any missing dependencies in
`site_scons/deps/*.py`. You can also attempt to use custom installations of
any library by changing the search paths in the `deps/*.py`.

5. To see advanced build options including cross compiling type `scons -h`

# Android Build Instructions for robolib

The android configuration only builds a subset of libraries and programs
that enable the supported Andoid robolib client side apps (available in a
separate repo).

1. Download Android Studio and install the latest NDK version compatible with your device
(the same NDK must be used to build the pre-requisites and this repo).

2. First you need to build and install libav for the Android architectures you will target.
You should acquire the matching libav version for the Ubuntu dustribution you will be using on
the server as follows:
```
$ mkdir ffmpeg_builds
$ cd ffmpeg_builds
$ apt source libavformat-dev
$ cp -r ffmpeg-3.4.6/ ffmpeg-3.4.6_android_<arch-name>
```

You should then configure to cross-compile with the same NDK and architecture(s) as you will use later e.g.:

```
$ ./configure --disable-everything --enable-shared --enable-small \
--enable-cross-compile \
--sysroot=$HOME/Android/Sdk/ndk/21.1.6352462/toolchains/llvm/prebuilt/linux-x86_64/sysroot/ \
--cross-prefix=$HOME/Android/Sdk/ndk/21.1.6352462/toolchains/llvm/prebuilt/linux-x86_64/bin/arm-linux-androideabi- \
--cc=$HOME/Android/Sdk/ndk/21.1.6352462/toolchains/llvm/prebuilt/linux-x86_64/bin/armv7a-linux-androideabi29-clang \
--arch=armv7-a \
--target-os=android \
--disable-symver \
--prefix=$HOME/development/install/android/armeabi-v7a/ffmpeg \
--extra-cflags='-mfloat-abi=softfp -O3 -fno-integrated-as' \
--enable-muxer=m4v --enable-demuxer=m4v \
--enable-encoder=mpeg4 --enable-decoder=mpeg4
$ make
```

Configure NOTES:
a) -fno-integrated-as is a workaround for an ASM bug when compiling with clang.
b) If you get errors about text relocations at runtime you need to rebuild with the flag --disable-asm (gives poor performance).

3. Edit the Android specific paths in `site_scons/compilers.py` to point to your NDK then build:

```
$ scons --platform=android
```
