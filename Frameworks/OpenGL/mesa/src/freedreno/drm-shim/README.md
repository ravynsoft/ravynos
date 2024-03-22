### freedreno_noop backend

This implements the minimum of msm in order to make shader-db work.
The submit ioctl is stubbed out to not execute anything.

Export `MESA_LOADER_DRIVER_OVERRIDE=msm
LD_PRELOAD=$prefix/lib/libfreedreno_noop_drm_shim.so`.

By default, a630 is exposed.  The chip can be selected an enviornment
variable like `FD_GPU_ID=307"
