### nouveau_noop backend

This implements the minimum of nouveau in order to make shader-db work.
The submit ioctl is stubbed out to not execute anything.

Export `MESA_LOADER_DRIVER_OVERRIDE=nouveau
LD_PRELOAD=$prefix/lib/libnouveau_noop_drm_shim.so`.

By default, GK110 is exposed.  The chip can be selected with an environment
variable like `NOUVEAU_CHIPSET=f0`. Some chips of note with the appropriate
chipset ids:

| ID  | Chip Name | Sample Marketing Name | Comment |
| --- | --------- | --------------------- | ------- |
| 30  | NV30      | GeForce FX 5500       | DX9     |
| 40  | NV40      | GeForce 6800          | DX9c, better control flow |
| 50  | G80       | GeForce 8800 GTS      | SM 1.0, 128 regs |
| 84  | G84       | GeForce 8600 GS       | SM 1.1  |
| a0  | GT200     | GeForce GTX 280       | SM 1.2 + fp64 |
| a3  | GT215     | GeForce GT 240        | DX10.1 ops |
| c0  | GF100     | GeForce GTX 480       | SM 2.0, 64 regs |
| e4  | GK104     | GeForce GTX 680       | SM 3.0, 64 regs |
| f0  | GK110     | GeForce GTX 780       | SM 3.5, 256 regs |
| 117 | GM107     | GeForce GTX 750       | SM 5.0  |
| 124 | GM204     | GeForce GTX 980       | SM 5.2  |
| 134 | GP104     | GeForce GTX 1080      | SM 6.0  |
| 140 | GV100     | TITAN V               | SM 7.0  |
| 162 | TU102     | GeForce RTX 2080      | SM 7.5  |
