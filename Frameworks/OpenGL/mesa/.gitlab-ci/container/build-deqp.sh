#!/usr/bin/env bash
# shellcheck disable=SC2086 # we want word splitting

# When changing this file, you need to bump the following
# .gitlab-ci/image-tags.yml tags:
# DEBIAN_X86_64_TEST_ANDROID_TAG
# DEBIAN_X86_64_TEST_GL_TAG
# DEBIAN_X86_64_TEST_VK_TAG
# KERNEL_ROOTFS_TAG

set -ex -o pipefail

DEQP_VERSION=vulkan-cts-1.3.7.0

git config --global user.email "mesa@example.com"
git config --global user.name "Mesa CI"
git clone \
    https://github.com/KhronosGroup/VK-GL-CTS.git \
    -b $DEQP_VERSION \
    --depth 1 \
    /VK-GL-CTS
pushd /VK-GL-CTS

mkdir -p /deqp

echo "dEQP base version $DEQP_VERSION" > /deqp/version-log

# Patches to VulkanCTS may come from commits in their repo (listed in
# cts_commits_to_backport) or patch files stored in our repo (in the patch
# directory `$OLDPWD/.gitlab-ci/container/patches/` listed in cts_patch_files).
# Both list variables would have comments explaining the reasons behind the
# patches.

cts_commits_to_backport=(
    # Take multiview into account for task shader inv. stats
    22aa3f4c59f6e1d4daebd5a8c9c05bce6cd3b63b

    # Remove illegal mesh shader query tests
    2a87f7b25dc27188be0f0a003b2d7aef69d9002e

    # Relax fragment shader invocations result verifications
    0d8bf6a2715f95907e9cf86a86876ff1f26c66fe

    # Fix several issues in dynamic rendering basic tests
    c5453824b498c981c6ba42017d119f5de02a3e34
)

for commit in "${cts_commits_to_backport[@]}"
do
  PATCH_URL="https://github.com/KhronosGroup/VK-GL-CTS/commit/$commit.patch"
  echo "Apply patch to VK-GL-CTS from $PATCH_URL"
  curl -L --retry 4 -f --retry-all-errors --retry-delay 60 $PATCH_URL | \
    git am -
done

cts_patch_files=(
  # Android specific patches.
  build-deqp_Allow-running-on-Android-from-the-command-line.patch
  build-deqp_Android-prints-to-stdout-instead-of-logcat.patch
)

for patch in "${cts_patch_files[@]}"
do
  echo "Apply patch to VK-GL-CTS from $patch"
  git am < $OLDPWD/.gitlab-ci/container/patches/$patch
done

echo "The following local patches are applied on top:" >> /deqp/version-log
git log --reverse --oneline $DEQP_VERSION.. --format=%s | sed 's/^/- /' >> /deqp/version-log

# --insecure is due to SSL cert failures hitting sourceforge for zlib and
# libpng (sigh).  The archives get their checksums checked anyway, and git
# always goes through ssh or https.
python3 external/fetch_sources.py --insecure

# Save the testlog stylesheets:
cp doc/testlog-stylesheet/testlog.{css,xsl} /deqp
popd

pushd /deqp

if [ "${DEQP_TARGET}" != 'android' ]; then
    # When including EGL/X11 testing, do that build first and save off its
    # deqp-egl binary.
    cmake -S /VK-GL-CTS -B . -G Ninja \
        -DDEQP_TARGET=x11_egl_glx \
        -DCMAKE_BUILD_TYPE=Release \
        $EXTRA_CMAKE_ARGS
    ninja modules/egl/deqp-egl
    mv /deqp/modules/egl/deqp-egl /deqp/modules/egl/deqp-egl-x11

    cmake -S /VK-GL-CTS -B . -G Ninja \
        -DDEQP_TARGET=wayland \
        -DCMAKE_BUILD_TYPE=Release \
        $EXTRA_CMAKE_ARGS
    ninja modules/egl/deqp-egl
    mv /deqp/modules/egl/deqp-egl /deqp/modules/egl/deqp-egl-wayland
fi

cmake -S /VK-GL-CTS -B . -G Ninja \
      -DDEQP_TARGET=${DEQP_TARGET:-default} \
      -DCMAKE_BUILD_TYPE=Release \
      $EXTRA_CMAKE_ARGS

# Make sure `default` doesn't silently stop detecting one of the platforms we care about
if [ "${DEQP_TARGET}" = 'default' ]; then
  grep -q DEQP_SUPPORT_WAYLAND=1 build.ninja
  grep -q DEQP_SUPPORT_X11=1 build.ninja
  grep -q DEQP_SUPPORT_XCB=1 build.ninja
fi

mold --run ninja

if [ "${DEQP_TARGET}" = 'android' ]; then
    mv /deqp/modules/egl/deqp-egl /deqp/modules/egl/deqp-egl-android
fi

# Copy out the mustpass lists we want.
mkdir /deqp/mustpass
for mustpass in $(< /VK-GL-CTS/external/vulkancts/mustpass/main/vk-default.txt) ; do
    cat /VK-GL-CTS/external/vulkancts/mustpass/main/$mustpass \
        >> /deqp/mustpass/vk-master.txt
done

if [ "${DEQP_TARGET}" != 'android' ]; then
    cp \
        /deqp/external/openglcts/modules/gl_cts/data/mustpass/gles/aosp_mustpass/3.2.6.x/*.txt \
        /deqp/mustpass/.
    cp \
        /deqp/external/openglcts/modules/gl_cts/data/mustpass/egl/aosp_mustpass/3.2.6.x/egl-master.txt \
        /deqp/mustpass/.
    cp \
        /deqp/external/openglcts/modules/gl_cts/data/mustpass/gles/khronos_mustpass/3.2.6.x/*-master.txt \
        /deqp/mustpass/.
    cp \
        /deqp/external/openglcts/modules/gl_cts/data/mustpass/gl/khronos_mustpass/4.6.1.x/*-master.txt \
        /deqp/mustpass/.
    cp \
        /deqp/external/openglcts/modules/gl_cts/data/mustpass/gl/khronos_mustpass_single/4.6.1.x/*-single.txt \
        /deqp/mustpass/.

    # Save *some* executor utils, but otherwise strip things down
    # to reduct deqp build size:
    mkdir /deqp/executor.save
    cp /deqp/executor/testlog-to-* /deqp/executor.save
    rm -rf /deqp/executor
    mv /deqp/executor.save /deqp/executor
fi

# Remove other mustpass files, since we saved off the ones we wanted to conventient locations above.
rm -rf /deqp/external/openglcts/modules/gl_cts/data/mustpass
rm -rf /deqp/external/vulkancts/modules/vulkan/vk-master*
rm -rf /deqp/external/vulkancts/modules/vulkan/vk-default

rm -rf /deqp/external/openglcts/modules/cts-runner
rm -rf /deqp/modules/internal
rm -rf /deqp/execserver
rm -rf /deqp/framework
find . -depth \( -iname '*cmake*' -o -name '*ninja*' -o -name '*.o' -o -name '*.a' \) -exec rm -rf {} \;
${STRIP_CMD:-strip} external/vulkancts/modules/vulkan/deqp-vk
${STRIP_CMD:-strip} external/openglcts/modules/glcts
${STRIP_CMD:-strip} modules/*/deqp-*
du -sh ./*
rm -rf /VK-GL-CTS
popd
