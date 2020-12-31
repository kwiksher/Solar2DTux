* jetson boot bug 

	https://forums.developer.nvidia.com/t/jetson-nano-blank-screen-during-and-after-boot/160356/27

	ctrl-alt-F5 and get a log in prompt. Not sure why f5 and not f2-f4

	```
	systemctl restart gdm3
	systemctl status gdm3
	```


* rsync sysroot

```
	rsync -rzLR --safe-links \
      nano@jetson:/usr/lib/aarch64-linux-gnu \
      nano@jetson:/usr/lib/gcc/aarch64-linux-gnu \
      nano@jetson:/usr/include \
      nano@jetson:/lib/aarch64-linux-gnu \
      nano@jetson:/usr/share/pkgconfig \
      sysroot/ 
```

* toolchain

	GCC Tool Chain for 64-bit BSP 

	https://developer.nvidia.com/embedded/dlc/l4t-gcc-7-3-1-toolchain-64-bit

	https://developer.nvidia.com/embedded/downloads

	https://forums.developer.nvidia.com/t/cross-compiling-c-c-code-for-jetson-nano-in-ubuntu-18-04-in-a-host-pc-x86-x64/73473


* build wx

	/Users/ymmtny/Documents/work/Solar2DTux/external/wx/wxWidgets-3.1.4/gtk-build/setup-llvm.sh
	--without-libtiff 

	* crtbegin crtend

	```
	SYSROOT="$HOME/Documents/work/sysroot_nano"
	TARGET=aarch64-linux-gnu
	LIBPATH=${SYSROOT/lib/gcc/aarch64-unknown-linux-gnu/7

	ln -s ${LIBPATH}/crtbegin.o ${SYSROOT}/usr/lib/crtbegin.o
	ln -s ${LIBPATH}/crtend.o ${SYSROOT}/usr/lib/crtend.o
	ln -s ${LIBPATH}/usr/lib/gcc/${TARGET}/8/crtbeginS.o
	ln -s ${LIBPATH}/usr/lib/gcc/${TARGET}/8/crtendS.o

	ln -s ${SYSROOT/usr/lib/aarch64-linux-gnu ${SYSROOT/usr/lib/aarch64         
	```


	* compile error

	fatal error: use of overloaded operator '[]' is ambiguous (with operand types 'wxString' and 'unsigned int')

	https://stackoverflow.com/questions/23600736/cant-build-wxwidgets-on-cygwin-ambiguous-overload-for-operator-basedll-ape
	(size_t)

	* igonore the error at the end

	```
	(if test -f utils/wxrc/Makefile ; then cd utils/wxrc && /Applications/Xcode.app/Contents/	Developer/usr/bin/make all ; fi)

		ld.lld: error: undefined symbol: wxEntry(int&, char**)
		>>> referenced by wxrc.cpp:257 (../../../utils/wxrc/wxrc.cpp:257)
		>>>               wxrc_wxrc.o:(main)

	```

* CryptpCPP

    cd external/CryptoPP/cryptopp-master 

	```
    make libcryptopp.a \
	BASE=$HOME/Documents/work/Solar2DTux/tools/llvm/bin \
	SYSROOT="$HOME/Documents/work/sysroot_nano" \
	TARGET=aarch64-linux-gnu \
	CXX="${BASE}/${TARGET}-clang++" CC="${BASE}/${TARGET}-clang" CXXFLAGS="-O2 -g3  -std=c++11"
	```

	```
    make install PREFIX=$(pwd)/../../../platform/linux \
	TARGET=aarch64-linux-gnu \
	CXX="${BASE}/${TARGET}-clang++" CC="${BASE}/${TARGET}-clang" CXXFLAGS="-O2 -g3  -std=c++11"
	```

* curl

    ```
    cd external/curl/curl-7.64.1
    
    ./configure --prefix=$(pwd)/../../../platform/linux \
		CPPFLAGS="-I$(pwd)/../../../platform/linux/include/openssl" \
		LDFLAGS="-L$(pwd)/../../../platform/linux/lib" \
		--with-ssl --disable-shared \
		BASE=$HOME/Documents/work/Solar2DTux/tools/llvm/bin \
		SYSROOT="$HOME/Documents/work/sysroot_nano" \
		TARGET=aarch64-linux-gnu \
		CXX="${BASE}/${TARGET}-clang++" CC="${BASE}/${TARGET}-clang" CXXFLAGS="-O2 -g3  -std=c++11" \
		--host=${TARGET}

    make
    make install
    ```


* openssl

	http://wiki.macchiatobin.net/tiki-index.php?page=OpenSSL+Installation+Guide

    ```
    cd external/openssl/openssl-1.1.1h
    
    ./config --prefix=$(pwd)/../../../platform/linux --openssldir=/usr/local/ssl \
		BASE=$HOME/Documents/work/Solar2DTux/tools/llvm/bin \
		SYSROOT="$HOME/Documents/work/sysroot_nano" \
		TARGET=aarch64-linux-gnu \
		CXX="${BASE}/${TARGET}-clang++" CC="${BASE}/${TARGET}-clang" CXXFLAGS="-O2 -g3  -std=c++11"

    make depend
    make all
    make install
    ```


* core pattern

	sudo sysctl -w kernel.core_pattern=/home/pi/work/tmp/core-%e-%s-%u-%g-%p-%t

* memory check with valgrind

	valgrind --leak-check=full ./Solar2DSimulator
