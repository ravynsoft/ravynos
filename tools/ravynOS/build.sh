PKG_CONFIG_PATH=/usr/libdata/pkgconfig:/usr/local/libdata/pkgconfig
IGNORE_OSVERSION=yes
CIRRUS_WORKING_DIR=${CIRRUS_WORKING_DIR:-/usr/src}
PLATFORM=${PLATFORM:-$(uname -m).$(uname -p)}

install() {
    cd ${CIRRUS_WORKING_DIR}
    make -j$(sysctl -n hw.ncpu) MALLOC_PRODUCTION=1 WITHOUT_CLEAN=1 MK_LIB32=no COMPILER_TYPE=clang installworld
    make -j$(sysctl -n hw.ncpu) MK_LIB32=no KERNCONF=RAVYN COMPILER_TYPE=clang installkernel
    if [ -d drm-kmod ]; then
        COMPILER_TYPE=clang make -C drm-kmod install
    fi
    if [ -d neofetch ]; then
        gmake -C neofetch install
    fi
    if [ -d plutil ]; then
        gmake -C plutil install
    fi
    make COMPILER_TYPE=clang -f Makefile.ravynOS install
}


base_build() {
    cd ${CIRRUS_WORKING_DIR}
    make -C gnu/usr.bin/gmake all worldtmp_install COMPILER_TYPE=clang DESTDIR=/usr/obj/${CIRRUS_WORKING_DIR}/${PLATFORM}
    make -j$(sysctl -n hw.ncpu) MALLOC_PRODUCTION=1 WITHOUT_CLEAN=1 MK_LIB32=no COMPILER_TYPE=clang buildworld
    if [ $? -ne 0 ]; then exit $?; fi
}

kernel_build() {
    cd ${CIRRUS_WORKING_DIR}
    make -j$(sysctl -n hw.ncpu) MK_LIB32=no KERNCONF=RAVYN COMPILER_TYPE=clang buildkernel
    if [ $? -ne 0 ]; then exit $?; fi
}

drm_build() {
    cd ${CIRRUS_WORKING_DIR}
    make -j$(sysctl -n hw.ncpu) MK_LIB32=no KERNCONF=RAVYN COMPILER_TYPE=clang installkernel
    if [ $? -ne 0 ]; then exit $?; fi
    if [ ! -d drm-kmod ]; then
        git clone https://github.com/ravynsoft/drm-kmod.git
    fi
    COMPILER_TYPE=clang make -C drm-kmod 
    mkdir -p /usr/obj/${CIRRUS_WORKING_DIR}/${PLATFORM}/release/dist/kernel/boot/modules
    COMPILER_TYPE=clang make -C drm-kmod install DESTDIR=/usr/obj/${CIRRUS_WORKING_DIR}/${PLATFORM}/release/dist/kernel/
    if [ $? -ne 0 ]; then exit $?; fi
}

system_build() {
    cd ${CIRRUS_WORKING_DIR}
    #ln -sf ../sys /usr/obj/${CIRRUS_WORKING_DIR}/${PLATFORM}/tmp/sys
    make COMPILER_TYPE=clang -f Makefile.ravynOS prep
    if [ $? -ne 0 ]; then exit $?; fi
    cp -fv share/mk/* /usr/share/mk/
    make COMPILER_TYPE=clang -f Makefile.ravynOS
    if [ $? -ne 0 ]; then exit $?; fi
}

extras_build() {
    cd ${CIRRUS_WORKING_DIR}
    if [ ! -d neofetch ]; then
        git clone https://github.com/ravynsoft/neofetch.git
    fi
    cd neofetch && gmake DESTDIR=/usr/obj/${CIRRUS_WORKING_DIR}/${PLATFORM}/release/dist/ravynOS install
    if [ $? -ne 0 ]; then exit $?; fi
    cd ${CIRRUS_WORKING_DIR}
    if [ ! -d plutil ]; then
        git clone https://github.com/ravynsoft/plutil.git
    fi
    cd plutil
    git submodule update --init --recursive
    gmake && gmake DESTDIR=/usr/obj/${CIRRUS_WORKING_DIR}/${PLATFORM}/release/dist/ravynOS install
    if [ $? -ne 0 ]; then exit $?; fi
}

basepkg() {
    cd ${CIRRUS_WORKING_DIR}
    make -C release MK_LIB32=no NOSRC=true NOPORTS=true KERNCONF=RAVYN COMPILER_TYPE=clang kernel.txz
    if [ $? -ne 0 ]; then exit $?; fi
}

kernelpkg() {
    cd ${CIRRUS_WORKING_DIR}
    make -C release MK_LIB32=no NOSRC=true NOPORTS=true KERNCONF=RAVYN COMPILER_TYPE=clang base.txz
    if [ $? -ne 0 ]; then exit $?; fi
}

systempkg() {
    rm -f dist/ravynOS.txz
    tar cvJ -C /usr/obj/${CIRRUS_WORKING_DIR}/${PLATFORM}/release/dist/ravynOS --gid 0 --uid 0 -f $(pwd)/dist/ravynOS.txz .
    if [ $? -ne 0 ]; then exit $?; fi
}

isoalt() {
    cp -fv version.txt ISO/overlays/ramdisk/version
    mkdir -p /usr/local/furybsd/$(uname -m)/cache/$(head -1 version.txt)/base
    cp -fv /usr/obj/${CIRRUS_WORKING_DIR}/${PLATFORM}/release/base.txz /usr/obj/${CIRRUS_WORKING_DIR}/${PLATFORM}/release/kernel.txz ${CIRRUS_WORKING_DIR}/dist/ravynOS.txz /usr/local/furybsd/$(uname -m)/cache/$(head -1 version.txt)/base/
    cd ISO; RAVYNOS=${CIRRUS_WORKING_DIR} ./build.sh ravynOS ravynOS_$(head -1 ../version.txt)
}

iso_build() {
    basepkg
    kernelpkg
    systempkg
    isoalt
}

while ! [ "z$1" = "z" ]; do
    case "$1" in
        base) base_build ;;
        kernel) kernel_build ;;
        system) system_build ;;
        extras) extras_build ;;
        iso) iso_build ;;
        basepkg) basepkg ;;
        kernelpkg) kernelpkg ;;
        systempkg) systempkg ;;
        isoalt) isoalt ;;
        all) kernel_build; base_build; system_build; extras_build; iso_build ;;
    esac
    shift
done
