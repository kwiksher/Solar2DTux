# Cross Compiling On OSX

* rsync sysroot to your work directory

```
rsync -rzLR --safe-links \
	nano@jetson:/usr/lib/aarch64-linux-gnu \
	nano@jetson:/usr/lib/gcc/aarch64-linux-gnu \
	nano@jetson:/usr/include \
	nano@jetson:/lib/aarch64-linux-gnu \
	nano@jetson:/usr/share/pkgconfig \
	sysroot_ubuntu/ 
```

---
## GCC 7.5 Binutils


* https://stackoverflow.com/questions/25634708/gnu-linker-equivalent-command-in-os-x
* https://wiki.osdev.org/GCC_Cross-Compiler#OS_X_Users

	```
	# GMP、MPFR、MPC 
	brew install gmp
	brew install mpfr
	brew install libmpc

	# Not use Mac' GCC(clang). use gcc 7.5 

	brew install gcc@7

	export CC=/usr/local/bin/gcc-7 
	export LD=/usr/local/bin/gcc-7

	export PREFIX="/usr/local/aarch64-linux-gnu"
	export TARGET=aarch64-linux-gnu
	export PATH="$PREFIX/bin:$PATH"

	# binutils

	mkdir /tmp/src
	cd /tmp/src
	curl -O http://ftp.gnu.org/gnu/binutils/binutils-2.35.1.tar.gz
	tar xf binutils-2.35.1.tar.gz
	mkdir binutils-build
	cd binutils-build
	../binutils-2.35.1/configure --target=$TARGET --enable-interwork --enable-multilib --disable-nls --disable-werror --prefix=$PREFIX 2>&1 | tee configure.log
	sudo make all install 2>&1 | tee make.log

	# Update libiconv

	cd /tmp/src
	curl -O https://ftp.gnu.org/pub/gnu/libiconv/libiconv-1.16.tar.gz
	tar xf libiconv-1.16.tar.gz
	cd libiconv-1.16
	./configure -prefix=/usr/local
	make
	make install

	# GCC aarch64

	cd /tmp/src
	curl -O https://ftp.gnu.org/gnu/gcc/gcc-7.5.0/gcc-7.5.0.tar.gz
	tar xf gcc-7.5.0.tar.gz
	mkdir gcc-build
	cd gcc-build
	../gcc-5.3.0/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --disable-libssp --enable-languages=c --without-headers --with-gmp=/usr/local --with-mpfr=/usr/local --with-mpfr=/usr/local --with-libiconv-prefix=/usr/local
	make all-gcc 
	make all-target-libgcc 
	sudo make install-gcc 
	sudo make install-target-libgcc
	```
---
## (Alternative) toolchain from nvidia (X86-x&4)

GCC Tool Chain for 64-bit BSP 

* https://developer.nvidia.com/embedded/dlc/l4t-gcc-7-3-1-toolchain-64-bit

* https://developer.nvidia.com/embedded/downloads

* https://forums.developer.nvidia.com/t/cross-compiling-c-c-code-for-jetson-nano-in-ubuntu-18-04-in-a-host-pc-x86-x64/73473


---
## Clang

https://clang.llvm.org/get_started.html

```
wget http://releases.llvm.org/11.0.0/clang+llvm-11.0.0-x86_64-apple-darwin.tar.xz
tar -xzvf clang+llvm-11.0.0-x86_64-apple-darwin.tar.xz -C "WORK_DIR" --strip-components=1
```


references

* https://llvm.org/docs/CommandGuide/index.html
* https://blog.meinside.dev/Build-Cross-Compile-Tools-for-Raspberry-Pi-on-macOS/

	```
	#clang+llvm
	wget http://releases.llvm.org/6.0.0/clang+llvm-6.0.0-x86_64-apple-darwin.tar.xz
	tar -xzvf clang+llvm-6.0.0-x86_64-apple-darwin.tar.xz -C "raspbian-sdk/prebuilt" --strip-components=1
	
	#binutils
	wget http://ftp.gnu.org/gnu/binutils/binutils-2.30.tar.xz
	tar -xzvf binutils-2.30.tar.xz
	cd binutils-2.30
	brew install coreutils
	./configure --prefix="`/usr/local/bin/realpath ../raspbian-sdk/prebuilt`" \
		--target=arm-linux-gnueabihf \
		--enable-gold=yes \
		--enable-ld=yes \
		--enable-targets=arm-linux-gnueabihf \
		--enable-multilib \
		--enable-interwork \
		--disable-werror \
		--quiet
	make && make install
	```

* https://medium.com/@haraldfernengel/cross-compiling-c-c-from-macos-to-raspberry-pi-in-2-easy-steps-23f391a8c63

---
## CMake

```
brew install cmake
```

cd Solar2DTux/platform/linux/cmkae-build-Release

* builder
* car
* console
* lua
* output
* simulator
* template_player

cd console

../../configure

```
BASEDIR=$(dirname "$0")

cmake -DLLVM_ENABLE_PROJECTS=clang -G "Unix Makefiles" \
	-DCMAKE_CXX_COMPILER=/usr/local/opt/llvm/bin/clang++ \
	-DCMAKE_C_COMPILER=/usr/local/opt/llvm/bin/clang \
	-DBASE=/usr/local/opt/llvm/bin \
	-DSYSROOT="$HOME/Documents/work/sysroot_ubuntu" \
	-DTARGET=aarch64-linux-gnu \
	-DISYSTEM_0=/usr/include/c++/7.5.0 \
	-DISYSTEM_1=/usr/include/aarch64-linux-gnu \
	-DISYSTEM_2=/usr/include/aarch64-linux-gnu/c++/7.5.0 \
	-DTOOLCHAIN=/usr/local/aarch64-linux-gnu-gcc \
	-DLIB_1=/usr/lib/gcc/aarch64-linux-gnu/7.5.0 \
	-DLIB_2=/usr/lib/aarch64-linux-gnu \
	-DCMAKE_OSX_SYSROOT=$HOME/Documents/work/sysroot_ubuntu \
	-DCONFIG_NAME="Release" \
	$BASEDIR/.
```

CMakeLists.txt 

>please enable one of console, simulator, car, lua, simualtor, template_player

```
include(${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.console)
#include(${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.simulator)

...
...

```
--- 
## Cross Compile: Dependencies of Solar2D

BASE=$HOME/Documents/work/Solar2DTux/tools/llvm/bin
export CXX="${BASE}/${TARGET}-clang++" 
export CC="${BASE}/${TARGET}-clang" 
export CXXFLAGS="-DNDEBUG -O2 -g3  -std=c++11"

* CryptoPP

	https://www.cryptopp.com/wiki/ARM_Embedded_(Command_Line)

	```
	export IS_ARM_EMBEDDED=1
	make -f GNUmakefile-cross libcryptopp.a
	make -f GNUmakefile-cross install-lib PREFIX=$(pwd)/../../../platform/linux
	```

* CryptpCPP

    cd external/CryptoPP/cryptopp-master 

	```
    make libcryptopp.a \
	SYSROOT="$HOME/Documents/work/sysroot_ubuntu" \
	TARGET=aarch64-linux-gnu
	```

	```
    make install PREFIX=$(pwd)/../../../platform/linux \
	TARGET=aarch64-linux-gnu
	```

* Curl

    ```
    cd external/curl/curl-7.64.1
    
    ./configure --prefix=$(pwd)/../../../platform/linux \
		CPPFLAGS="-I$(pwd)/../../../platform/linux/include/openssl" \
		LDFLAGS="-L$(pwd)/../../../platform/linux/lib" \
		--with-ssl --disable-shared \
		SYSROOT="$HOME/Documents/work/sysroot_ubuntu" \
		--host=aarch64-linux-gnu

    make
    make install
    ```


* Openssl

	http://wiki.macchiatobin.net/tiki-index.php?page=OpenSSL+Installation+Guide

    ```
    cd external/openssl/openssl-1.1.1h
    
    ./config --prefix=$(pwd)/../../../platform/linux --openssldir=/usr/local/ssl SYSROOT="$HOME/Documents/work/sysroot_ubuntu" \
		TARGET=aarch64-linux-gnu

    make depend
    make all
    make install
    ```

* wxWidgets

	cd /Users/ymmtny/Documents/work/Solar2DTux/external/wx/wxWidgets-3.1.4/gtk-build/
	
	source configure-aarch64-llvm.sh

	```
	BASE=$HOME/Documents/work/Solar2DTux/tools/llvm/bin
	SYSROOT="$HOME/Documents/work/sysroot_ubuntu"
	TARGET=aarch64-linux-gnu

	export CXX="${BASE}/${TARGET}-clang++"  
	export CC="${BASE}/${TARGET}-clang" 
	export CXXFLAGS="-O2 -g3  -std=c++11"  
	export CFLAGS="" 
	export PKG_CONFIG_PATH=${SYSROOT}/usr/lib/${TARGET}/pkgconfig:${SYSROOT}/usr/share/pkgconfig 
	export AR="/usr/local/opt/llvm/bin/llvm-ar rcu" 

	../configure --prefix=$(pwd)/../../../../platform/linux --with-opengl \
	--disable-shared --enable-webview --enable-webviewwebkit --enable-mediactrl --enable-graphics_ctx --with-libtiff=builtin --host=${TARGET} 
	```

	* (Alternative) configure-aarch64-linux-gnu.sh

	   gcc 7.5 aarch64-linux-gnu instead of clang

		```
		BASE=$HOME/Documents/work/Solar2DTux/tools/gcc/bin
		export CXX="${BASE}/${TARGET}-g++"  
		export CC="${BASE}/${TARGET}-gcc" 
		...
		...
		```
	
	* Solar2DTux/tools/llvm/bin

		* aarch64-linux-gnu-clang++
		* aarch64-linux-gnu-clang

		```
		BASE=/usr/local/opt/llvm/bin
		SYSROOT="$HOME/Documents/work/sysroot_ubuntu"
		TARGET=aarch64-linux-gnu
		TOOLCHAIN=/usr/local/aarch64-linux-gnu-gcc
		LIBPATH="${SYSROOT}/usr/lib/gcc/${TARGET}/7.5.0"
		LIBARMPATH="${SYSROOT}/usr/lib/aarch64-linux-gnu"
		exec env LIBPATH="${LIBPATH}" \
		"${BASE}/clang++" --target=${TARGET} \
			--sysroot="${SYSROOT}" \
			-isysroot "${SYSROOT}" \
			-isystem "${SYSROOT}/usr/include/c++/7.5.0" \
			-isystem "${SYSROOT}/usr/include/aarch64-linux-gnu/c++/7.5.0" \
			-isystem "${SYSROOT}/usr/lib/aarch64-linux-gnu/glib-2.0/include/" \
			-I"${SYSROOT}/usr/include/gtk-3.0" \
			-I"${SYSROOT}/usr/include" \
			-I"${SYSROOT}/usr/include/cairo" \
			-I"${SYSROOT}/usr/include/gdk-pixbuf-2.0" \
			-I"${SYSROOT}/usr/include/atk-1.0" \
			-I"${SYSROOT}/usr/include/cairo" \
			-I"${SYSROOT}/usr/include/glib-2.0" \
			-I"${SYSROOT}/usr/include/pango-1.0" \
			-I"${SYSROOT}/usr/include/freetype2" \
			-I"${SYSROOT}/usr/include/gtk-3.0/unix-print" \
			-I"${SYSROOT}/usr/lib/aarch64-linux-gnu/glib-2.0/include" \
			-I"${SYSROOT}/usr/include/gstreamer-1.0" \
			-I"${SYSROOT}/usr/include/webkitgtk-4.0" \
			-I"${SYSROOT}/usr/include/libsoup-2.4" \
			-rpath "${LIBARMPATH}" \
			-L"${LIBPATH}" \
			-fuse-ld=lld \
			--gcc-toolchain="${TOOLCHAIN}" \
			-v \
			"$@"
		```
		

	* compile error

		fatal error: use of overloaded operator '[]' is ambiguous (with operand types 'wxString' and 'unsigned int')

		https://stackoverflow.com/questions/23600736/cant-build-wxwidgets-on-cygwin-ambiguous-overload-for-operator-basedll-ape

		* cast it with **(size_t)**

	* caveats:crtbegin crtend not found error?

		```
		SYSROOT="$HOME/Documents/work/sysroot_ubuntu"
		TARGET=aarch64-linux-gnu
		LIBPATH=${SYSROOT/lib/gcc/aarch64-unknown-linux-gnu/7

		ln -s ${LIBPATH}/crtbegin.o ${SYSROOT}/usr/lib/crtbegin.o
		ln -s ${LIBPATH}/crtend.o ${SYSROOT}/usr/lib/crtend.o
		ln -s ${LIBPATH}/usr/lib/gcc/${TARGET}/8/crtbeginS.o
		ln -s ${LIBPATH}/usr/lib/gcc/${TARGET}/8/crtendS.o

		ln -s ${SYSROOT/usr/lib/aarch64-linux-gnu ${SYSROOT/usr/lib/aarch64         
		```



	* caveats:igonore the error at the end, seems the test failed but ok?

		```
		(if test -f utils/wxrc/Makefile ; then cd utils/wxrc && /Applications/Xcode.app/Contents/	Developer/usr/bin/make all ; fi)

			ld.lld: error: undefined symbol: wxEntry(int&, char**)
			>>> referenced by wxrc.cpp:257 (../../../utils/wxrc/wxrc.cpp:257)
			>>>               wxrc_wxrc.o:(main)

		```
