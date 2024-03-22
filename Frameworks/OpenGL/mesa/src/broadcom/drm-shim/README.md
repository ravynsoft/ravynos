### v3d_noop backend

This implements the minimum of v3d in order to make shader-db work.
The submit ioctl is stubbed out to not execute anything.

Export `MESA_LOADER_DRIVER_OVERRIDE=v3d
LD_PRELOAD=$prefix/lib/libv3d_noop_drm_shim.so`.  This will be a V3D
4.2 device.

### vc4_noop backend

This implements the minimum of vc4 in order to make shader-db work.
The submit ioctl is stubbed out to not execute anything.

Export `MESA_LOADER_DRIVER_OVERRIDE=vc4
LD_PRELOAD=$prefix/lib/libvc4_noop_drm_shim.so`.  This will be a VC4
2.1 device.

