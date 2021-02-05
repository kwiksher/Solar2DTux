SYSROOT="/mnt/e/Ubuntu/sysroot_arm32v7"
TARGET="arm-linux-gnueabihf"
BASE=/usr/bin
CPLUS=7.5.0

#TOOLCHAIN=

CROSS=(" --sysroot=${SYSROOT}"
"-I${SYSROOT}/usr/include"
"-I${SYSROOT}/usr/include/gtk-3.0"
"-I${SYSROOT}/usr/include/cairo"
"-I${SYSROOT}/usr/include/gdk-pixbuf-2.0"
"-I${SYSROOT}/usr/include/atk-1.0"
"-I${SYSROOT}/usr/include/cairo"
"-I${SYSROOT}/usr/include/glib-2.0"
"-I${SYSROOT}/usr/include/pango-1.0"
"-I${SYSROOT}/usr/include/freetype2"
"-I${SYSROOT}/usr/include/gtk-3.0/unix-print"
"-I${SYSROOT}/usr/lib/${TARGET}/glib-2.0/include"
"-L${SYSROOT}/usr/lib/${TARGET}"
"-L${SYSROOT}/usr/lib/gcc/${TARGET}/${CPLUS}"
"-Wl,-rpath=${SYSROOT}/usr/lib/${TARGET}:${SYSROOT}/lib/${TARGET}"
"-v" )

#echo $CROSS

_CXX=(${BASE}/${TARGET}-g++  ${CROSS[@]})  
_CC=(${BASE}/${TARGET}-gcc ${CROSS[@]})

#_CXX=(/mnt/c/Users/ymmtny/ProgramFiles/LLVM/bin/clang++.exe --target=${TARGET} ${CROSS[@]} -save-temps -fuse-ld=lld)  
#_CC=(/mnt/c/Users/ymmtny/ProgramFiles/LLVM/bin/clang.exe --target=${TARGET} ${CROSS[@]} -save-temps -fuse-ld=lld)


export CC="${_CC[@]}"  
export CXX="${_CXX[@]}" 
export CXXFLAGS="-O2 -g3  -std=c++11"  
export CFLAGS="" 
export PKG_CONFIG_PATH=${SYSROOT}/usr/lib/${TARGET}/pkgconfig:${SYSROOT}/usr/share/pkgconfig 
#export AR="/mnt/c/Users/ymmtny/ProgramFiles/LLVM/bin/llvm-ar.exe" 
#export AROPTIONS="rcu"

../configure --prefix=$(pwd)/../../../../platform/linux --with-opengl --disable-shared --enable-monolithic --enable-webview --enable-webviewwebkit --enable-mediactrl --enable-graphics_ctx  --host=${TARGET}  


#--with-libtiff=builtin
#--without-libtiff