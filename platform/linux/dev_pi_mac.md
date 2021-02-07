# Cross Compiling On OSX

raspberry pi 4 

* kernal 64 bit and 32 bit user land
* arm-linux-gnueabihf

docker

* docker pull arm32v7/ubuntu:18.04

	docker run it. We will copy sysroot to /home/work which is mounted to a host directory where you copy the sysroot files.

	```
	docker container run  -v $HOME/Documents/work/sysroot_ubuntu:/home/work --name ubuntu_arm32v7 arm32v7/ubuntu:18.04 /bin/bash
	```

* apt update
* source ./download_solar2d_dependencies.sh

	* ref  platform/linux/Solar2DSimulator/install.sh
	```
	sudo apt-get install build-essential -y
	sudo apt-get install libopenal-dev -y
	sudo apt-get install libgtk-3-dev -y
	sudo apt-get install libpng-dev -y
	sudo apt-get install zlib1g-dev -y
	sudo apt-get install libgstreamer1.0-dev -y
	sudo apt-get install libgstreamer-plugins-base1.0-dev -y
	sudo apt-get install libjpeg-dev -y
	sudo apt-get install libssl-dev -y
	sudo apt-get install libvorbis-dev -y
	sudo apt-get install libogg-dev -y
	sudo apt-get install uuid-dev -y
	sudo apt-get install libxxf86vm-dev -y
	sudo apt-get install libwebkit2gtk-4.0-37 -y
	sudo apt-get install libwebkit2gtk-4.0-dev -y
	sudo apt-get install libgstreamer1.0-0-dbg -y
	sudo apt-get install libzopfli1 -y
	sudo apt-get install openjdk-8-jdk-headless -y
	sudo apt-get install openjdk-8-jre-headless -y
	sudo apt-get install unzip -y
	sudo apt-get install git -y
	sudo apt-get install p7zip -y
	sudo apt-get install p7zip-full -y
	sudo apt-get install lua5.1 -y
	sudo apt-get install gradle -y
	```

* crtbegin.o and crtend.o 
	```
	ln -s \usr\lib\gcc\arm-linux-gnueabihf\7.5.0\crtbegin.o \usr\lib\gcc\arm-linux-gnueabihf
	ln -s \usr\lib\gcc\arm-linux-gnueabihf\7.5.0\crtend.o  \usr\lib\gcc\arm-linux-gnueabihf
		```

copy sysroot to 

```
cd /home/work
mkdir usr & cd usr
mkdir lib & cd lib
mkdir gcc 
cd ..
mkdir share
cd ..
mkdir lib
cd ..

cd usr/lib 
cp -rL /usr/lib/arm-linux-gnueabihf .
cd gcc
cp -rL /usr/lib/gcc/arm-linux-gnueabihf .
cd ../../../usr
cp -rL /usr/include .
cd ../lib
cp -rL /lib/arm-linux-gnueabihf .
cd ../usr/share
cp -rL /usr/share/pkgconfig .

```

Or rsync from a real device
```
rsync -rzLR --safe-links \
	pi@ubuntu:/usr/lib/arm-linux-gnueabihf \
	pi@ubuntu:/usr/lib/gcc/arm-linux-gnueabihf \
	pi@ubuntu:/usr/include \
	pi@ubuntu:/lib/arm-linux-gnueabihf \
	pi@ubuntu:/usr/share/pkgconfig \
	sysroot_ubuntu/ 
```

---
## CMake

```
brew install cmake
```

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




----
Solar2DTux/platform/linux

* builder/
* car/
* console/
* lua/
* simulator/
* template_player/

* CMakeLists.txt 

	please enable one of console, simulator, car, lua, simualtor, template_player

	```
	include(${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.console)
	#include(${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.simulator)

	...
	...

	```

run configure

```
cd console
../configure
```
* cofigure
	```
	BASEDIR=$(dirname "$0")
	BASE=$HOME/Documents/work/Solar2DTux/tools/llvm/bin
	TARGET=arm-linux-gnueabihf
	CPLUS=7.5.0

	cmake -G "Unix Makefiles" \
		-DCMAKE_CXX_COMPILER="${BASE}/${TARGET}-clang++" \
		-DCMAKE_C_COMPILER="${BASE}/${TARGET}-clang" \
		-DSYSROOT="$HOME/Documents/work/sysroot_ubuntu" \
		-DTARGET=${TARGET} \
		-DISYSTEM_0=/usr/include/c++/${CPLUS} \
		-DISYSTEM_1=/usr/include/${TARGET} \
		-DISYSTEM_2=/usr/include/${TARGET}/c++/${CPLUS} \
		-DTOOLCHAIN=/usr/bin/${TARGET}-gcc \
		-DLIB_1=/usr/lib/gcc/${TARGET}/${CPLUS} \
		-DLIB_2=/usr/lib/${TARGET} \
		-DLIB_3=/lib/${TARGET} \
		-DCONFIG_NAME="Release" \
		$BASEDIR/.

	```

--- 
## Cross Compile: Dependencies of Solar2D

```
BASE=$HOME/Documents/work/Solar2DTux/tools/llvm/bin
export CXX="${BASE}/${TARGET}-clang++" 
export CC="${BASE}/${TARGET}-clang" 
export CXXFLAGS="-DNDEBUG -O2 -g3  -std=c++11"
```

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
	TARGET=arm-linux-gnueabihf
	```

	```
    make install PREFIX=$(pwd)/../../../platform/linux \
	TARGET=arm-linux-gnueabihf
	```

* Curl

    ```
    cd external/curl/curl-7.64.1
    
    ./configure --prefix=$(pwd)/../../../platform/linux \
		CPPFLAGS="-I$(pwd)/../../../platform/linux/include/openssl" \
		LDFLAGS="-L$(pwd)/../../../platform/linux/lib" \
		--with-ssl --disable-shared \
		SYSROOT="$HOME/Documents/work/sysroot_ubuntu" \
		--host=arm-linux-gnueabihf

    make
    make install
    ```


* Openssl

	http://wiki.macchiatobin.net/tiki-index.php?page=OpenSSL+Installation+Guide

    ```
    cd external/openssl/openssl-1.1.1h
    
    ./config --prefix=$(pwd)/../../../platform/linux --openssldir=/usr/local/ssl SYSROOT="$HOME/Documents/work/sysroot_ubuntu" \
		TARGET=arm-linux-gnueabihf

    make depend
    make all
    make install
    ```

* wxWidgets

	cd /Users/ymmtny/Documents/work/Solar2DTux/external/wx/wxWidgets-3.1.4/gtk-build/
	
	source configure-arm-linux-gnueabihf-llvm.sh

	```
	BASE=$HOME/Documents/work/Solar2DTux/tools/llvm/bin
	SYSROOT="$HOME/Documents/work/sysroot_ubuntu"
	TARGET=arm-linux-gnueabihf

	export CXX="${BASE}/${TARGET}-clang++"  
	export CC="${BASE}/${TARGET}-clang" 
	export CXXFLAGS="-O2 -g3  -std=c++11"  
	export CFLAGS="" 
	export PKG_CONFIG_PATH=${SYSROOT}/usr/lib/${TARGET}/pkgconfig:${SYSROOT}/usr/share/pkgconfig 
	export AR="/usr/local/opt/llvm/bin/llvm-ar rcu" 

	../configure --prefix=$(pwd)/../../../../platform/linux --with-opengl \
	--disable-shared --enable-webview --enable-webviewwebkit --enable-mediactrl --enable-graphics_ctx --with-libtiff=builtin --host=${TARGET} 
	```

	* Solar2DTux/tools/llvm/bin

		* arm-linux-gnueabihf-clang++
		* arm-linux-gnueabihf-clang

		```
		BASE=/usr/local/opt/llvm/bin
		SYSROOT="$HOME/Documents/work/sysroot_ubuntu"
		TARGET=arm-linux-gnueabihf
		TOOLCHAIN=/usr/local/arm-linux-gnueabihf-gcc
		LIBPATH="${SYSROOT}/usr/lib/gcc/${TARGET}/7.5.0"
		LIBARMPATH="${SYSROOT}/usr/lib/arm-linux-gnueabihf"
		exec env LIBPATH="${LIBPATH}" \
		"${BASE}/clang++" --target=${TARGET} \
			--sysroot="${SYSROOT}" \
			-isysroot "${SYSROOT}" \
			-isystem "${SYSROOT}/usr/include/c++/7.5.0" \
			-isystem "${SYSROOT}/usr/include/arm-linux-gnueabihf/c++/7.5.0" \
			-isystem "${SYSROOT}/usr/lib/arm-linux-gnueabihf/glib-2.0/include/" \
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
			-I"${SYSROOT}/usr/lib/arm-linux-gnueabihf/glib-2.0/include" \
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
		TARGET=arm-linux-gnueabihf
		LIBPATH=${SYSROOT/lib/gcc/aarch64-unknown-linux-gnu/7

		ln -s ${LIBPATH}/crtbegin.o ${SYSROOT}/usr/lib/crtbegin.o
		ln -s ${LIBPATH}/crtend.o ${SYSROOT}/usr/lib/crtend.o
		ln -s ${LIBPATH}/usr/lib/gcc/${TARGET}/8/crtbeginS.o
		ln -s ${LIBPATH}/usr/lib/gcc/${TARGET}/8/crtendS.o

		ln -s ${SYSROOT/usr/lib/arm-linux-gnueabihf ${SYSROOT/usr/lib/aarch64         
		```



	* caveats:igonore the error at the end, seems the test failed but ok?

		```
		(if test -f utils/wxrc/Makefile ; then cd utils/wxrc && /Applications/Xcode.app/Contents/	Developer/usr/bin/make all ; fi)

			ld.lld: error: undefined symbol: wxEntry(int&, char**)
			>>> referenced by wxrc.cpp:257 (../../../utils/wxrc/wxrc.cpp:257)
			>>>               wxrc_wxrc.o:(main)

		```

---
## GCC 7.5 Binutils for aarch64 (Nvidia Jetson)

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

	export PREFIX="/usr/local/aarch64-linux-gnu
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
	../gcc-7.5.0/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --disable-libssp --enable-languages=c --without-headers --with-gmp=/usr/local --with-mpfr=/usr/local --with-mpfr=/usr/local --with-libiconv-prefix=/usr/local
	make all-gcc 
	make all-target-libgcc 
	sudo make install-gcc 
	sudo make install-target-libgcc
	```
