# DRM shim - Fake GEM kernel drivers in userspace for CI

On CI systems where we don't control the kernel, it would be nice to
be able to present either no-op GEM devices (for shader-db runs) or
simulator-backed GEM devices (for testing against a software simulator
or FPGA).  This lets us do that by intercepting libc calls and
exposing render nodes.

## Limitations

- Doesn't know how to handle DRM fds getting passed over the wire from
  X11 (Could we use kmsro to support the X11 case?).
- libc interception is rather glibc-specific and fragile.
- Can easily break gdb if the libc interceptor code is what's broken.
  (ulimit -c unlimited and doing gdb on the core after the fact can
  help)

## Using

You choose the backend by setting `LD_PRELOAD` to the shim you want.
Since this will effectively fake another DRM device to your system,
you may need some work on your userspace to get your test application
to use it if it's not the only DRM device present.  Setting
`DRM_SHIM_DEBUG=1` in the environment will print out what path the
shim initialized on.

For piglit tests, you can set:

```
PIGLIT_PLATFORM=gbm
WAFFLE_GBM_DEVICE=<path from DRM_SHIM_DEBUG>
```

See your drm-shim backend's README for details on how to use it.
