IGNORE_OSVERSION=yes
CIRRUS_WORKING_DIR=${CIRRUS_WORKING_DIR:-$PWD}
PLATFORM=${PLATFORM:-$(uname -m).$(uname -p)}
PREFIX=${PREFIX:-/usr}
HW_CPUS=$(sysctl -n hw.ncpu)
CORES=${CORES:-${HW_CPUS}}
BUILDROOT=/usr/obj/${CIRRUS_WORKING_DIR}/${PLATFORM}
TMPBIN=${BUILDROOT}/tmp/usr/bin
TMPLEGACY=${BUILDROOT}/tmp/legacy/bin

export PREFIX

install() {
    cd ${CIRRUS_WORKING_DIR}
    make -j${CORES} MALLOC_PRODUCTION=1 WITHOUT_CLEAN=1 MK_LIB32=no COMPILER_TYPE=clang installworld
    make -j${CORES} MK_LIB32=no KERNCONF=RAVYN COMPILER_TYPE=clang installkernel
    if [ -d drm-kmod ]; then
        COMPILER_TYPE=clang make -C drm-kmod install
    fi
    if [ -d neofetch ]; then
        ${TMPLEGACY}/gmake -C neofetch install
    fi
    if [ -d plutil ]; then
        ${TMPLEGACY}/gmake -C plutil install
    fi
    make COMPILER_TYPE=clang -f Makefile.ravynOS install
}


base_build() {
    cd ${CIRRUS_WORKING_DIR}
    make -j${CORES} MALLOC_PRODUCTION=1 WITHOUT_CLEAN=1 \
	MK_LIB32=no MK_LLVM_TARGET_X86=yes \
	MK_LLVM_TARGET_ARM=yes MK_LLVM_TARGET_AARCH64=yes \
	MK_LLVM_TARGET_RISCV=no MK_LLVM_TARGET_POWERPC=no \
	MK_LLVM_TARGET_MIPS=no MK_LLVM_TARGET_BPF=no \
	COMPILER_TYPE=clang buildworld
    if [ $? -ne 0 ]; then exit $?; fi
}

kernel_build() {
    cd ${CIRRUS_WORKING_DIR}
    make -j${CORES} MK_LIB32=no KERNCONF=RAVYN WITHOUT_CLEAN=1 COMPILER_TYPE=clang buildkernel
    if [ $? -ne 0 ]; then exit $?; fi
}

drm_build() {
    cd ${CIRRUS_WORKING_DIR}
    # Is this install actually needed?
    #make -j${CORES} MK_LIB32=no KERNCONF=RAVYN WITHOUT_CLEAN=1 COMPILER_TYPE=clang installkernel
    #if [ $? -ne 0 ]; then exit $?; fi
    if [ ! -d drm-kmod ]; then
        git clone https://github.com/ravynsoft/drm-kmod.git
    fi
    COMPILER_TYPE=clang SYSDIR=${CIRRUS_WORKING_DIR}/sys make -C drm-kmod 
    mkdir -p /usr/obj/${CIRRUS_WORKING_DIR}/${PLATFORM}/release/dist/kernel/boot/modules
    COMPILER_TYPE=clang make -C drm-kmod install DESTDIR=/usr/obj/${CIRRUS_WORKING_DIR}/${PLATFORM}/release/dist/kernel/
    if [ $? -ne 0 ]; then exit $?; fi
}

system_build() {
    cd ${CIRRUS_WORKING_DIR}
    echo "CIRRUS_CI=${CIRRUS_CI}"
    if [ "x${CIRRUS_CI}" = "xtrue" ]; then
	echo "Symlinking OBJTOP/usr/include/machine for CI"
        ln -sf ${CIRRUS_WORKING_DIR}/sys/$(uname -m)/include \
	/usr/obj/${CIRRUS_WORKING_DIR}/${PLATFORM}/tmp/usr/include/machine
    fi
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
    cp -fv ${CIRRUS_WORKING_DIR}/neofetch/neofetch \
	${BUILDROOT}/release/dist/ravynOS/usr/bin/
    cp -fv ${CIRRUS_WORKING_DIR}/neofetch/neofetch.1 \
	${BUILDROOT}/release/dist/ravynOS/usr/share/man/man1/
    chmod 755 ${BUILDROOT}/release/dist/ravynOS/usr/bin/neofetch
    if [ $? -ne 0 ]; then exit $?; fi
    cd ${CIRRUS_WORKING_DIR}
    if [ ! -d plutil ]; then
        git clone https://github.com/ravynsoft/plutil.git
    fi
    cd plutil
    git submodule update --init --recursive
    PATH=${PATH}:${TMPBIN} \
	CMAKE_FLAGS="-DLIBXML2_LIBRARY=${BUILDROOT}/tmp/usr/lib/libxml2.so -DLIBXML2_INCLUDE_DIR=${BUILDROOT}/tmp/usr/include/libxml2" \
	${TMPLEGACY}/gmake
    PATH=${PATH}:${TMPBIN} ${TMPLEGACY}/gmake \
	DESTDIR=${BUILDROOT}/release/dist/ravynOS install
    if [ $? -ne 0 ]; then exit $?; fi
    cd ${CIRRUS_WORKING_DIR}
}

cleanpkg() {
    rm -f dist/ravynOS.txz
    for pkg in kernel kernel-dbg base base-dbg tests
    do \
	rm -f ${BUILDROOT}/release/${pkg}.txz
    done
}

kernelpkg() {
    cd ${CIRRUS_WORKING_DIR}
    rm -f ${BUILDROOT}/release/kernel.txz
    rm -f ${BUILDROOT}/release/kernel-dbg.txz
    make -C release MK_LIB32=no NOSRC=true NOPORTS=true KERNCONF=RAVYN COMPILER_TYPE=clang kernel.txz
    if [ $? -ne 0 ]; then exit $?; fi
}

basepkg() {
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

echo ravynOS Build Tool [Prefix ${PREFIX} Cores ${CORES} Platform ${PLATFORM}]
while ! [ "z$1" = "z" ]; do
    case "$1" in
        base) base_build ;;
        kernel) kernel_build ;;
        system) system_build ;;
        extras) extras_build ;;
	drm) drm_build ;;
        iso) iso_build ;;
        basepkg) basepkg ;;
        kernelpkg) kernelpkg ;;
        systempkg) systempkg ;;
        isoalt) isoalt ;;
	cleanpkg) cleanpkg ;;
	install) install ;;
        all) kernel_build; drm_build; base_build; system_build; extras_build; iso_build ;;
    esac
    shift
done
