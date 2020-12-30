SYSROOT="$HOME/Documents/work/sysroot_nano"
TARGET=aarch64-linux-gnu
LIBPATH=$HOME/Documents/work/gcc-4.8.5-aarch64/lib/gcc/aarch64-unknown-linux-gnu/4.8.5

ln -s ${LIBPATH}/crtbegin.o ${SYSROOT}/usr/lib/crtbegin.o
ln -s ${LIBPATH}/crtend.o ${SYSROOT}/usr/lib/crtend.o
ln -s ${LIBPATH}/usr/lib/gcc/${TARGET}/8/crtbeginS.o
ln -s ${LIBPATH}/usr/lib/gcc/${TARGET}/8/crtendS.o
