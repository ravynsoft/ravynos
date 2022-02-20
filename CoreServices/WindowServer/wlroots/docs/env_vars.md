wlroots reads these environment variables

# wlroots specific

* *WLR_BACKENDS*: comma-separated list of backends to use (available backends:
  libinput, drm, wayland, x11, headless)
* *WLR_NO_HARDWARE_CURSORS*: set to 1 to use software cursors instead of
  hardware cursors
* *WLR_XWAYLAND*: specifies the path to an Xwayland binary to be used (instead
  of following shell search semantics for "Xwayland")
* *WLR_RENDERER*: forces the creation of a specified renderer (available
  renderers: gles2, pixman, vulkan)
* *WLR_RENDER_DRM_DEVICE*: specifies the DRM node to use for
  hardware-accelerated renderers.

## DRM backend

* *WLR_DRM_DEVICES*: specifies the DRM devices (as a colon separated list)
  instead of auto probing them. The first existing device in this list is
  considered the primary DRM device.
* *WLR_DRM_NO_ATOMIC*: set to 1 to use legacy DRM interface instead of atomic
  mode setting
* *WLR_DRM_NO_MODIFIERS*: set to 1 to always allocate planes without modifiers,
  this can fix certain modeset failures because of bandwidth restrictions.

## Headless backend

* *WLR_HEADLESS_OUTPUTS*: when using the headless backend specifies the number
  of outputs

## libinput backend

* *WLR_LIBINPUT_NO_DEVICES*: set to 1 to not fail without any input devices

## Wayland backend

* *WLR_WL_OUTPUTS*: when using the wayland backend specifies the number of outputs

## X11 backend

* *WLR_X11_OUTPUTS*: when using the X11 backend specifies the number of outputs

## gles2 renderer

* *WLR_RENDERER_ALLOW_SOFTWARE*: allows the gles2 renderer to use software
  rendering

# Generic

* *DISPLAY*: if set probe X11 backend in `wlr_backend_autocreate`
* *WAYLAND_DISPLAY*, *WAYLAND_SOCKET*: if set probe Wayland backend in
  `wlr_backend_autocreate`
* *XCURSOR_PATH*: directory where xcursors are located
* *XDG_SESSION_ID*: if set, session ID used by the logind session
