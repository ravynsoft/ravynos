### radeon_noop backend

This implements the minimum of the radeon kernel driver in order to make shader-db work.
The submit ioctl is stubbed out to not execute anything.

Export `MESA_LOADER_DRIVER_OVERRIDE=r300
LD_PRELOAD=$prefix/lib/libradeon_noop_drm_shim.so`. (or r600 for r600-class HW)

By default, rv515 is exposed.  The chip can be selected an environment
variable like `RADEON_GPU_ID=CAYMAN` or `RADEON_GPU_ID=0x6740`.

### amdgpu_noop backend

This implements the minimum of the amdgpu kernel driver.  The submit ioctl is
stubbed out to not execute anything.

Export `LD_PRELOAD=$prefix/lib/libamdgpu_noop_drm_shim.so`.

To specify the device to expose, set the environment variable `AMDGPU_GPU_ID`
to

 - `renoir` to expose a `CHIP_RENOIR` device
 - `raven` to expose a `CHIP_RAVEN` device
 - `stoney` to expose a `CHIP_STONEY` device

Further names follow the `CHIP_*` enum values. By default, the `CHIP_RENOIR`
device is exposed.

To add a new device, `amdgpu_devices.c` needs to be modified.
`amdgpu_dump_states` can be used to dump the relevant states from a real
device.
