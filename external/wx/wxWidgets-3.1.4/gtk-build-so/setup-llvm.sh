BASE=$HOME/Documents/work/Solar2DTux/tools/llvm/bin
SYSROOT="$HOME/Documents/work/sysroot"
TARGET=arm-linux-gnueabihf

../configure --prefix=$(pwd)/../../../../platform/linux --with-opengl --enable-monolithic --enable-webview --enable-webviewwebkit --enable-mediactrl --enable-graphics_ctx --without-libtiff --host=${TARGET} CXX="${BASE}/${TARGET}-clang++"  CC="${BASE}/${TARGET}-clang" CXXFLAGS="-O2 -g3  -std=c++11"  CFLAGS="" PKG_CONFIG_PATH=${SYSROOT}/usr/lib/arm-linux-gnueabihf/pkgconfig:${SYSROOT}/usr/share/pkgconfig AR="/usr/local/opt/llvm/bin/llvm-ar" AROPTIONS="rcu"

