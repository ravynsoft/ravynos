#!/usr/bin/env bash
# shellcheck disable=SC2086 # we want word splitting

# When changing this file, you need to bump the following
# .gitlab-ci/image-tags.yml tags:
# DEBIAN_BUILD_TAG

set -ex

EPHEMERAL=(
    autoconf
    rdfind
    unzip
)

apt-get install -y --no-remove "${EPHEMERAL[@]}"

# Fetch the NDK and extract just the toolchain we want.
ndk=$ANDROID_NDK
curl -L --retry 4 -f --retry-all-errors --retry-delay 60 \
  -o $ndk.zip https://dl.google.com/android/repository/$ndk-linux.zip
unzip -d / $ndk.zip "$ndk/toolchains/llvm/*"
rm $ndk.zip
# Since it was packed as a zip file, symlinks/hardlinks got turned into
# duplicate files.  Turn them into hardlinks to save on container space.
rdfind -makehardlinks true -makeresultsfile false /${ndk}/
# Drop some large tools we won't use in this build.
find /${ndk}/ -type f \( -iname '*clang-check*' -o -iname '*clang-tidy*' -o -iname '*lldb*' \) -exec rm -f {} \;

sh .gitlab-ci/container/create-android-ndk-pc.sh /$ndk zlib.pc "" "-lz" "1.2.3" $ANDROID_SDK_VERSION

sh .gitlab-ci/container/create-android-cross-file.sh /$ndk x86_64-linux-android x86_64 x86_64 $ANDROID_SDK_VERSION
sh .gitlab-ci/container/create-android-cross-file.sh /$ndk i686-linux-android x86 x86 $ANDROID_SDK_VERSION
sh .gitlab-ci/container/create-android-cross-file.sh /$ndk aarch64-linux-android aarch64 armv8 $ANDROID_SDK_VERSION
sh .gitlab-ci/container/create-android-cross-file.sh /$ndk arm-linux-androideabi arm armv7hl $ANDROID_SDK_VERSION armv7a-linux-androideabi

for arch in \
        x86_64-linux-android \
        i686-linux-android \
        aarch64-linux-android \
        arm-linux-androideabi ; do
    EXTRA_MESON_ARGS="--cross-file=/cross_file-$arch.txt --libdir=lib/$arch -Dnouveau=disabled -Dintel=disabled" \
    . .gitlab-ci/container/build-libdrm.sh
done

rm -rf $LIBDRM_VERSION

export LIBELF_VERSION=libelf-0.8.13
curl -L --retry 4 -f --retry-all-errors --retry-delay 60 \
  -O https://fossies.org/linux/misc/old/$LIBELF_VERSION.tar.gz

# Not 100% sure who runs the mirror above so be extra careful
if ! echo "4136d7b4c04df68b686570afa26988ac ${LIBELF_VERSION}.tar.gz" | md5sum -c -; then
    echo "Checksum failed"
    exit 1
fi

tar -xf ${LIBELF_VERSION}.tar.gz
cd $LIBELF_VERSION

# Work around a bug in the original configure not enabling __LIBELF64.
autoreconf

for arch in \
        x86_64-linux-android \
        i686-linux-android \
        aarch64-linux-android \
        arm-linux-androideabi ; do

    ccarch=${arch}
    if [ "${arch}" ==  'arm-linux-androideabi' ]
    then
       ccarch=armv7a-linux-androideabi
    fi

    export CC=/${ndk}/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-ar
    export CC=/${ndk}/toolchains/llvm/prebuilt/linux-x86_64/bin/${ccarch}${ANDROID_SDK_VERSION}-clang
    export CXX=/${ndk}/toolchains/llvm/prebuilt/linux-x86_64/bin/${ccarch}${ANDROID_SDK_VERSION}-clang++
    export LD=/${ndk}/toolchains/llvm/prebuilt/linux-x86_64/bin/${arch}-ld
    export RANLIB=/${ndk}/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-ranlib

    # The configure script doesn't know about android, but doesn't really use the host anyway it
    # seems
    ./configure --host=x86_64-linux-gnu  --disable-nls --disable-shared \
                --libdir=/usr/local/lib/${arch}
    make install
    make distclean
done

cd ..
rm -rf $LIBELF_VERSION

apt-get purge -y "${EPHEMERAL[@]}"
