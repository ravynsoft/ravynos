Android
=======

Mesa hardware drivers can be built for Android one of two ways: built
into the Android OS using the ndk-build build system on older versions
of Android, or out-of-tree using the Meson build system and the
Android NDK.

The ndk-build build system has proven to be hard to maintain, as one
needs a built Android tree to build against, and it has never been
tested in CI.  The Meson build system flow is frequently used by
Chrome OS developers for building and testing Android drivers.

Building using the Android NDK
------------------------------

Download and install the NDK using whatever method you normally would.
Then, create your Meson cross file to use it, something like this
``~/.local/share/meson/cross/android-aarch64`` file:

.. code-block:: ini

    [binaries]
    ar = 'NDKDIR/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android-ar'
    c = ['ccache', 'NDKDIR/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android29-clang']
    cpp = ['ccache', 'NDKDIR/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android29-clang++', '-fno-exceptions', '-fno-unwind-tables', '-fno-asynchronous-unwind-tables', '-static-libstdc++']
    c_ld = 'lld'
    cpp_ld = 'lld'
    strip = 'NDKDIR/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android-strip'
    # Android doesn't come with a pkg-config, but we need one for Meson to be happy not
    # finding all the optional deps it looks for.  Use system pkg-config pointing at a
    # directory we get to populate with any .pc files we want to add for Android
    pkgconfig = ['env', 'PKG_CONFIG_LIBDIR=NDKDIR/pkgconfig', '/usr/bin/pkg-config']

    [host_machine]
    system = 'linux'
    cpu_family = 'arm'
    cpu = 'armv8'
    endian = 'little'

Now, use that cross file for your Android build directory (as in this
one cross-compiling the turnip driver for a stock Pixel phone)

.. code-block:: console

    meson setup build-android-aarch64 \
        --cross-file android-aarch64 \
	-Dplatforms=android \
	-Dplatform-sdk-version=26 \
	-Dandroid-stub=true \
	-Dgallium-drivers= \
	-Dvulkan-drivers=freedreno \
	-Dfreedreno-kmds=kgsl
    meson compile -C build-android-aarch64

Replacing Android drivers on stock Android
------------------------------------------

The vendor partition with the drivers is normally mounted from a
read-only disk image on ``/vendor``.  To be able to replace them for
driver development, we need to unlock the device and remount
``/vendor`` read/write.

.. code-block:: console

    adb disable-verity
    adb reboot
    adb remount -R

Now you can replace drivers as in:

.. code-block:: console

    adb push build-android-aarch64/src/freedreno/vulkan/libvulkan_freedreno.so /vendor/lib64/hw/vulkan.sdm710.so

Note this command doesn't quite work because libvulkan wants the
SONAME to match.  For now, in turnip we have been using a hack to the
meson.build to change the SONAME.

Replacing Android drivers on Chrome OS
--------------------------------------

Chrome OS's ARC++ is an Android container with hardware drivers inside
of it.  The vendor partition with the drivers is normally mounted from
a read-only squashfs image on disk.  For doing rapid driver
development, you don't want to regenerate that image.  So, we'll take
the existing squashfs image, copy it out on the host, and then use a
bind mount instead of a loopback mount so we can update our drivers
using scp from outside the container.

On your device, you'll want to make ``/`` read-write.  ssh in as root
and run:

.. code-block:: console

    crossystem dev_boot_signed_only=0
    /usr/share/vboot/bin/make_dev_ssd.sh --remove_rootfs_verification --partitions 4
    reboot

Then, we'll switch Android from using an image for ``/vendor`` to using a
bind-mount from a directory we control.

.. code-block:: console

    cd /opt/google/containers/android/
    mkdir vendor-ro
    mount -o loop vendor.raw.img vendor-ro
    cp -a vendor-ro vendor-rw
    emacs config.json

In the ``config.json``, you want to find the block for ``/vendor`` and
change it to::

            {
                "destination": "/vendor",
                "type": "bind",
                "source": "/opt/google/containers/android/vendor-rw",
                "options": [
                    "bind",
                    "rw"
                ]
            },

Now, restart the UI to do a full reload:

.. code-block:: console

    restart ui

At this point, your android container is restarted with your new
bind-mount ``/vendor``, and if you use ``android-sh`` to shell into it
then the ``mount`` command should show::

    /dev/root on /vendor type ext2 (rw,seclabel,relatime)

Now, replacing your DRI driver with a new one built for Android should
be a matter of:

.. code-block:: console

    scp msm_dri.so $HOST:/opt/google/containers/android/vendor-rw/lib64/dri/

You can do your build of your DRI driver using ``emerge-$BOARD
arc-mesa-freedreno`` (for example) if you have a source tree with
ARC++, but it should also be possible to build using the NDK as
described above.  There are currently rough edges with this, for
example the build will require that you have your arc-libdrm build
available to the NDK, assuming you're building anything but the
Freedreno Vulkan driver for KGSL.  You can mostly put things in place
with:

.. code-block:: console

    scp $HOST:/opt/google/containers/android/vendor-rw/lib64/libdrm.so \
        NDKDIR/sysroot/usr/lib/aarch64-linux-android/lib/

    ln -s \
        /usr/include/xf86drm.h \
	/usr/include/libsync.h \
	/usr/include/libdrm \
	NDKDIR/sysroot/usr/include/

It seems that new invocations of an application will often reload the
DRI driver, but depending on the component you're working on you may
find you need to reload the whole Android container.  To do so without
having to log in to Chrome again every time, you can just kill the
container and let it restart:

.. code-block:: console

    kill $(cat /run/containers/android-run_oci/container.pid )
