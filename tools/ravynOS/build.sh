CIRRUS_WORKING_DIR=${CIRRUS_WORKING_DIR:-$PWD}
PLATFORM=${PLATFORM:-$(uname -m).$(uname -p)}
PREFIX=${PREFIX:-/usr}
HW_CPUS=$(sysctl -n hw.ncpu)
CORES=${CORES:-${HW_CPUS}}
BUILDROOT=/usr/obj/${CIRRUS_WORKING_DIR}/${PLATFORM}
TMPBIN=${BUILDROOT}/tmp/usr/bin
TMPLEGACY=${BUILDROOT}/tmp/legacy/bin

__MAKE_CONF=$PWD/tools/ravynOS/make.conf

export PREFIX __MAKE_CONF CIRRUS_WORKING_DIR PLATFORM

install() {
    cd ${CIRRUS_WORKING_DIR}
    make -j${CORES} installworld
    make -j${CORES} installkernel
    if [ -d drm-kmod ]; then
        make -C drm-kmod install
    fi
    if [ -d neofetch ]; then
        ${TMPLEGACY}/gmake -C neofetch install
    fi
    if [ -d plutil ]; then
        ${TMPLEGACY}/gmake -C plutil install
    fi
    make -f Makefile.ravynOS install

    while read -r LINE; do
        option="$(echo "$LINE" | cut -d= -f1)"
        if grep -q -v -E "^${option}=" /boot/loader.conf; then
            echo "$LINE" >> /boot/loader.conf
        fi
    done <<END
        cryptodev_load="YES"
        zfs_load="YES"
        beastie_disable="YES"
        autoboot_delay="3"
        vfs.root.mountfrom.options="rw"
        vfs.root.mountfrom="zfs:ravynOS/ROOT/default"
END

}


base_build() {
    cd ${CIRRUS_WORKING_DIR}
    make -j${CORES} buildworld
    if [ $? -ne 0 ]; then exit $?; fi
}

kernel_build() {
    cd ${CIRRUS_WORKING_DIR}
    make -j${CORES} buildkernel
    if [ $? -ne 0 ]; then exit $?; fi
}

drm_build() {
    cd ${CIRRUS_WORKING_DIR}
    # Is this install actually needed?
    #make -j${CORES} installkernel
    #if [ $? -ne 0 ]; then exit $?; fi
    if [ ! -d drm-kmod ]; then
        git clone https://github.com/ravynsoft/drm-kmod.git
    fi
    SYSDIR=${CIRRUS_WORKING_DIR}/sys make -C drm-kmod 
    mkdir -p /usr/obj/${CIRRUS_WORKING_DIR}/${PLATFORM}/release/dist/kernel/boot/modules
    SYSDIR=${CIRRUS_WORKING_DIR}/sys make -C drm-kmod install DESTDIR=/usr/obj/${CIRRUS_WORKING_DIR}/${PLATFORM}/release/dist/kernel/
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
    make -f Makefile.ravynOS prep
    if [ $? -ne 0 ]; then exit $?; fi
    cp -fv share/mk/* /usr/share/mk/
    make -f Makefile.ravynOS
    if [ $? -ne 0 ]; then exit $?; fi
}

extras_build() {
    cd ${CIRRUS_WORKING_DIR}
    if [ ! -d neofetch ]; then
        git clone https://github.com/ravynsoft/neofetch.git
    fi
    mkdir -p ${BUILDROOT}/release/dist/ravynOS/usr/bin/
    mkdir -p ${BUILDROOT}/release/dist/ravynOS/usr/share/man/man1/
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
    make -C release NOSRC=true NOPORTS=true kernel.txz
    if [ $? -ne 0 ]; then exit $?; fi
}

basepkg() {
    cd ${CIRRUS_WORKING_DIR}
    DISTDIR=/usr/obj/${CIRRUS_WORKING_DIR}/${PLATFORM}/release/dist
    mkdir -p ${DISTDIR}
    make -DNO_ROOT distributeworld NO_PORTS=true NOSRC=true DISTDIR=${DISTDIR}
    # Bootstrap etcupdate(8) database.
    sh ${CIRRUS_WORKING_DIR}/usr.sbin/etcupdate/etcupdate.sh extract -B \
        -m "make" -M "TARGET_ARCH=$(uname -m) TARGET=$(uname -p)" \
        -s ${CIRRUS_WORKING_DIR} -d "${DISTDIR}/base/var/db/etcupdate" \
        -L /dev/null -N
    # Package all components
    make packageworld NO_PORTS=true NOSRC=true DISTDIR=${DISTDIR}
    mv ${DISTDIR}/*.txz ${DISTDIR}/../
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
    cp -fv /usr/obj/${CIRRUS_WORKING_DIR}/${PLATFORM}/release/base.txz \
      /usr/obj/${CIRRUS_WORKING_DIR}/${PLATFORM}/release/kernel.txz \
      ${CIRRUS_WORKING_DIR}/dist/ravynOS.txz \
      /usr/local/furybsd/$(uname -m)/cache/$(head -1 version.txt)/base/
    cd ISO; RAVYNOS=${CIRRUS_WORKING_DIR} ./build.sh
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
