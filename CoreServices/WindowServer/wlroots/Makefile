LIB=    wlroots

MAJOR=          1
MINOR=          15
MICRO=          0
VERSION=        ${MAJOR}.${MINOR}.${MICRO}

SRCS=
STABLE=         xdg-shell \
                viewporter \
                presentation-time
STAGING=        drm-lease-v1 \
                xdg-activation-v1
UNSTABLE=       xdg-foreign-unstable-v1 \
                xdg-foreign-unstable-v2 \
                fullscreen-shell-unstable-v1 \
                pointer-constraints-unstable-v1 \
                keyboard-shortcuts-inhibit-unstable-v1 \
                tablet-unstable-v2 \
                linux-dmabuf-unstable-v1 \
                text-input-unstable-v3 \
                xdg-output-unstable-v1 \
                pointer-gestures-unstable-v1 \
                primary-selection-unstable-v1 \
                xdg-decoration-unstable-v1 \
                relative-pointer-unstable-v1 \
                idle-inhibit-unstable-v1

SCANNER=	${OBJTOP}/Frameworks/OpenGL/wayland/scanner/wayland-scanner

make-obj-dirs:
	mkdir -p types

.for f in ${STABLE} ${STAGING} ${UNSTABLE}
SRCS+=  ${f:T}.c
.endfor

.for f in ${STABLE}
${f:T}.c: ${.CURDIR}/../wayland-protocols/stable/${f}/${f}.xml
.endfor
.for f in ${STAGING}
${f:T}.c: ${.CURDIR}/../wayland-protocols/staging/${f:C/-v[0-9]*$//}/${f}.xml
.endfor
.for f in ${UNSTABLE}
${f:T}.c: ${.CURDIR}/../wayland-protocols/unstable/${f:C/-unstable.*$//}/${f}.xml
.endfor

PROTS!= ls ${.CURDIR}/protocol/*.xml
.for f in ${PROTS}
SRCS+=  ${f:T:R}.c
${f:T:R}.c: ${f}
.endfor

SRCS+=  types/wlr_compositor.c \
        types/wlr_touch.c \
        types/wlr_drm_lease_v1.c \
        types/wlr_xdg_activation_v1.c \
        types/wlr_pointer.c \
        types/wlr_keyboard_group.c \
        types/wlr_xdg_foreign_v1.c \
        types/wlr_export_dmabuf_v1.c \
        types/wlr_keyboard.c \
        types/xdg_shell/wlr_xdg_surface.c \
        types/xdg_shell/wlr_xdg_positioner.c \
        types/xdg_shell/wlr_xdg_popup.c \
        types/xdg_shell/wlr_xdg_shell.c \
        types/xdg_shell/wlr_xdg_toplevel.c \
        types/wlr_fullscreen_shell_v1.c \
        types/wlr_xdg_foreign_registry.c \
        types/wlr_xcursor_manager.c \
        types/wlr_output_damage.c \
        types/wlr_viewporter.c \
        types/wlr_output_power_management_v1.c \
        types/wlr_presentation_time.c \
        types/wlr_server_decoration.c \
        types/wlr_xdg_foreign_v2.c \
        types/wlr_pointer_constraints_v1.c \
        types/wlr_keyboard_shortcuts_inhibit_v1.c \
        types/wlr_output_layout.c \
        types/tablet_v2/wlr_tablet_v2.c \
        types/tablet_v2/wlr_tablet_v2_pad.c \
        types/tablet_v2/wlr_tablet_v2_tablet.c \
        types/tablet_v2/wlr_tablet_v2_tool.c \
        types/wlr_linux_dmabuf_v1.c \
        types/wlr_text_input_v3.c \
        types/wlr_xdg_output_v1.c \
        types/wlr_input_device.c \
        types/wlr_region.c \
        types/wlr_matrix.c \
        types/wlr_output_management_v1.c \
        types/data_device/wlr_data_device.c \
        types/data_device/wlr_data_source.c \
        types/data_device/wlr_data_offer.c \
        types/data_device/wlr_drag.c \
        types/wlr_virtual_pointer_v1.c \
        types/wlr_pointer_gestures_v1.c \
        types/wlr_drm.c \
        types/wlr_screencopy_v1.c \
        types/output/cursor.c \
        types/output/transform.c \
        types/output/render.c \
        types/output/output.c \
        types/wlr_idle.c \
        types/wlr_tablet_tool.c \
        types/scene/subsurface_tree.c \
        types/scene/output_layout.c \
        types/scene/wlr_scene.c \
        types/scene/xdg_shell.c \
        types/wlr_primary_selection_v1.c \
        types/seat/wlr_seat_touch.c \
        types/seat/wlr_seat_pointer.c \
        types/seat/wlr_seat_keyboard.c \
        types/seat/wlr_seat.c \
        types/wlr_primary_selection.c \
        types/wlr_xdg_decoration_v1.c \
        types/wlr_cursor.c \
        types/wlr_gamma_control_v1.c \
        types/wlr_foreign_toplevel_management_v1.c \
        types/wlr_tablet_pad.c \
        types/wlr_surface.c \
        types/wlr_switch.c \
        types/wlr_relative_pointer_v1.c \
        types/wlr_data_control_v1.c \
        types/wlr_virtual_keyboard_v1.c \
        types/wlr_layer_shell_v1.c \
        types/wlr_buffer.c \
        types/wlr_idle_inhibit_v1.c \
        types/wlr_input_inhibitor.c \
        util/box.c \
        util/log.c \
        util/global.c \
        util/array.c \
        util/shm.c \
        util/addon.c \
        util/time.c \
        util/token.c \
        util/signal.c \
        util/region.c \
        xcursor/wlr_xcursor.c \
        xcursor/xcursor.c \
        render/wlr_texture.c \
        render/wlr_renderer.c \
        render/dmabuf.c \
        render/gles2/pixel_format.c \
        render/gles2/renderer.c \
        render/gles2/shaders.c \
        render/gles2/texture.c \
        render/drm_format_set.c \
        render/swapchain.c \
        render/pixel_format.c \
        render/egl.c \
        render/allocator/shm.c \
        render/allocator/allocator.c \
        render/allocator/gbm.c \
        render/allocator/drm_dumb.c \
        render/pixman/renderer.c \
        render/pixman/pixel_format.c \
	backend/headless/backend.c \
	backend/headless/output.c \
	backend/headless/input_device.c \
        backend/wayland/tablet_v2.c \
        backend/wayland/seat.c \
        backend/wayland/backend.c \
        backend/wayland/output.c \
        backend/drm/backend.c \
        backend/drm/monitor.c \
        backend/drm/renderer.c \
        backend/drm/drm.c \
        backend/drm/util.c \
        backend/drm/properties.c \
        backend/drm/cvt.c \
        backend/drm/legacy.c \
        backend/drm/atomic.c \
        backend/backend.c \
        backend/libinput/pointer.c \
        backend/libinput/touch.c \
        backend/libinput/keyboard.c \
        backend/libinput/tablet_pad.c \
        backend/libinput/tablet_tool.c \
        backend/libinput/switch.c \
        backend/libinput/backend.c \
        backend/libinput/events.c \
        backend/session/session.c \
        backend/multi/backend.c
INCS=

MK_WERROR=	no
WARNS=	        1
CFLAGS+=	--sysroot=${OBJTOP}/tmp -B${OBJTOP}/tmp/usr/bin \
                -I${.CURDIR}/include -I${MAKEOBJDIR} -I${.CURDIR} \
                -I${OBJTOP}/Frameworks/OpenGL/wayland/src  \
                -I${SRCTOP}/Frameworks/OpenGL/wayland/src  \
                -I${SRCTOP}/Frameworks/OpenGL/libdrm  \
                -I${SRCTOP}/Frameworks/OpenGL/libdrm/include  \
                -I${SRCTOP}/Frameworks/OpenGL/libdrm/include/drm  \
                -I${SRCTOP}/Frameworks/AppKit/libxkbcommon/include \
                -I${.CURDIR}/../libinput/src -I${.CURDIR}/../seatd/include \
                -I${MAKEOBJDIR}/include \
                -I${SRCTOP}/Frameworks/OpenGL/mesa/include \
                -I${SRCTOP}/Frameworks/OpenGL/mesa/src/gbm/main \
                -DWLR_HAS_DRM_BACKEND -DWLR_HAS_LIBINPUT_BACKEND \
                -DWLR_HAS_GLES2_RENDERER -DWLR_HAS_X11_BACKEND=0 \
                -DWLR_HAS_VULKAN_RENDERER=0 -DWLR_HAS_XWAYLAND=0 \
                -DWLR_USE_UNSTABLE -DWLR_LITTLE_ENDIAN=1 \
	        -DWLR_BIG_ENDIAN=0 -DHAS_XCB_ERRORS=0 \
                -DWLR_VERSION_STR="${MAJOR}.${MINOR}.${MICRO}" \
                -DWLR_VERSION_MAJOR=${MAJOR} -DWLR_VERSION_MINOR=${MINOR} \
                -DWLR_VERSION_MICRO=${MICRO} -fPIC \
	        -Dhave_drm_backend=1 -Dhave_x11_backend=0 \
                -Dhave_libinput_backend=1 -Dhave_xwayland=0 \
                -Dhave_gles2_renderer=1 -Dhave_vulkan_renderer=0

LDFLAGS+=	-Wl,-R'$$ORIGIN/../Resources'

.SUFFIXES: .xml .c
.xml.c:
	${SCANNER} private-code ${.IMPSRC} ${.TARGET}
	${SCANNER} server-header ${.IMPSRC} ${.TARGET:R}-protocol.h
	${SCANNER} client-header ${.IMPSRC} ${.TARGET:R}-client-protocol.h

config.h::
	mkdir -p include/wlr
	touch include/wlr/config.h

version.h::
	mkdir -p include/wlr
	grep '^#define' ${.CURDIR}/include/wlr/version.h.in >include/wlr/version.h

.include <bsd.lib.mk>

${OBJS}: config.h version.h
beforebuild: make-obj-dirs obj
