# Cross Compiling On WSL Ubuntu

raspberry pi 4 

* kernal 64 bit and 32 bit user land
* arm-linux-gnueabihf

docker image location 

* use an external drive
    https://stackoverflow.com/questions/62441307/how-can-i-change-the-location-of-docker-images-when-using-docker-desktop-on-wsl2

    ```
    wsl --export docker-desktop-data "E:\Docker\wsl\data\docker-desktop-data.tar"
    wsl --unregister docker-desktop-data
    wsl --import docker-desktop-data "E:\Docker\wsl\data" "E:\Docker\wsl\data\docker-desktop-data.tar" --version 2
    ```

docker

* docker pull arm32v7/ubuntu:18.04

	docker run it. We will copy sysroot to /home/work which is mounted to a host directory where you copy the sysroot files.

	```
	docker container run  -v /mnt/e/Ubuntu/sysroot_arm32v7:/home/work --name ubuntu_arm32v7 arm32v7/ubuntu:18.04 /bin/bash
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
	sysroot_arm32v7/ 
```


---
##  GCC 7.5 arm linux gnueabihf

run normal Ubuntu. Just click ubuntu exe

* E:\Ubuntu_1804.2019.522.0_x64\ubuntu1804.exe

    > you can access the file system with File explorer with inputting **\\wsl$**

install cmake and  gcc for cross compiling

https://launchpad.net/ubuntu/bionic/+package/gcc-7-arm-linux-gnueabihf

```
sudo apt-get install gcc-7-arm-linux-gnueabihf g++-7-arm-linux-gnueabihf
```

cd Solar2DTux/platform/linux/cmake-build-Release

* builder
* car
* console
* lua
* output
* simulator
* template_player

```
cd console
../configure
```

* configure
    ```
    BASEDIR=$(dirname "$0")
    BASE=/usr/bin
    TARGET=arm-linux-gnueabihf
    CPLUS=7.5.0

    cmake -G "Unix Makefiles" \
        -DCMAKE_CXX_COMPILER="${BASE}/${TARGET}-g++" \
        -DCMAKE_C_COMPILER="${BASE}/${TARGET}-gcc" \
        -DSYSROOT="/mnt/e/Ubuntu/sysroot_arm32v7" \
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

CMakeLists.txt 

* please enable one of console, simulator, car, lua, simualtor, template_player

```
include(${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.console)
#include(${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.simulator)

...
...

```
--- 
## Cross Compile: Dependencies of Solar2D

```
BASE=/usr/bin
TARGET=arm-linux-gnueabihf
export CXX="${BASE}/${TARGET}-g++" 
export CC="${BASE}/${TARGET}-gcc" 
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
	SYSROOT="/mnt/e/Ubuntu/sysroot_arm32v7" \
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
		SYSROOT="/mnt/e/Ubuntu/sysroot_arm32v7" \
		--host=arm-linux-gnueabihf

    make
    make install
    ```


* Openssl

	http://wiki.macchiatobin.net/tiki-index.php?page=OpenSSL+Installation+Guide

    ```
    cd external/openssl/openssl-1.1.1h
    
    ./config --prefix=$(pwd)/../../../platform/linux --openssldir=/usr/local/ssl SYSROOT="/mnt/e/Ubuntu/sysroot_arm32v7" \
		TARGET=arm-linux-gnueabihf

    make depend
    make all
    make install
    ```

* wxWidgets

	cd Solar2DTux/external/wx/wxWidgets-3.1.4/gtk-build/
	
	source configure-arm-linu-gnueabihf-wsl.sh

	```
	BASE=/usr/bin
	SYSROOT="/mnt/e/Ubuntu/sysroot_arm32v7"
	TARGET=arm-linux-gnueabihf

	export CXX="${BASE}/${TARGET}-g++"  
	export CC="${BASE}/${TARGET}-gcc" 
	export CXXFLAGS="-O2 -g3  -std=c++11"  
	export CFLAGS="" 
	export PKG_CONFIG_PATH=${SYSROOT}/usr/lib/${TARGET}/pkgconfig:${SYSROOT}/usr/share/pkgconfig 
	#export AR="/usr/local/opt/llvm/bin/llvm-ar rcu" 

	../configure --prefix=$(pwd)/../../../../platform/linux --with-opengl \
	--disable-shared --enable-webview --enable-webviewwebkit --enable-mediactrl --enable-graphics_ctx --with-libtiff=builtin --host=${TARGET} 
	```

	* compile error

		fatal error: use of overloaded operator '[]' is ambiguous (with operand types 'wxString' and 'unsigned int')

		https://stackoverflow.com/questions/23600736/cant-build-wxwidgets-on-cygwin-ambiguous-overload-for-operator-basedll-ape

		* cast it with **(size_t)**


---
## (Alternative) 

toolchain from nvidia (X86-x&4). GCC Tool Chain for 64-bit BSP 

* https://developer.nvidia.com/embedded/dlc/l4t-gcc-7-3-1-toolchain-64-bit

* https://developer.nvidia.com/embedded/downloads

* https://forums.developer.nvidia.com/t/cross-compiling-c-c-code-for-jetson-nano-in-ubuntu-18-04-in-a-host-pc-x86-x64/73473


clang
	
* Solar2DTux/tools/llvm/bin

    * arm-linux-gnueabihf-clang++
    * arm-linux-gnueabihf-clang

    ```
    BASE=/usr/bin
    SYSROOT="/mnt/e/Ubuntu/sysroot_arm32v7"
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
