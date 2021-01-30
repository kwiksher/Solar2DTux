SYSROOT="/mnt/e/Jetson_Nano_2GB/sysroot_ubuntu"
TARGET="aarch64-linux-gnu"
BASE=/home/tux/gcc-linaro-7.3.1-2018.05-x86_64_aarch64-linux-gnu/bin

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
"-I${SYSROOT}/usr/lib/aarch64-linux-gnu/glib-2.0/include"
" -L${SYSROOT}/usr/lib/aarch64-linux-gnu"
"-L${SYSROOT}/usr/lib/gcc/aarch64-linux-gnu/7.5.0"
"-Wl,-rpath=${SYSROOT}/usr/lib/aarch64-linux-gnu:${SYSROOT}/lib/aarch64-linux-gnu"
" -v" )

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

../configure --prefix=$(pwd)/../../../../platform/linux --with-opengl --enable-monolithic --enable-webview --enable-webviewwebkit --enable-mediactrl --enable-graphics_ctx  --host=${TARGET}  


#--with-libtiff=builtin
#--without-libtiff