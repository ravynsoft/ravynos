STABLE=	        xdg-shell/xdg-shell.xml \
		viewporter/viewporter.xml \
		linux-dmabuf/linux-dmabuf-v1.xml \
		presentation-time/presentation-time.xml

STAGING=        ext-session-lock/ext-session-lock-v1.xml \
		drm-lease/drm-lease-v1.xml \
		xdg-activation/xdg-activation-v1.xml \
                content-type/content-type-v1.xml \
                cursor-shape/cursor-shape-v1.xml \
                ext-foreign-toplevel-list/ext-foreign-toplevel-list-v1.xml \
                ext-idle-notify/ext-idle-notify-v1.xml \
                ext-transient-seat/ext-transient-seat-v1.xml \
                fractional-scale/fractional-scale-v1.xml \
                security-context/security-context-v1.xml \
                single-pixel-buffer/single-pixel-buffer-v1.xml \
                tearing-control/tearing-control-v1.xml \
                xwayland-shell/xwayland-shell-v1.xml


UNSTABLE=       idle-inhibit/idle-inhibit-unstable-v1.xml \
		xwayland-keyboard-grab/xwayland-keyboard-grab-unstable-v1.xml \
		tablet/tablet-unstable-v2.xml \
		tablet/tablet-unstable-v1.xml \
		pointer-gestures/pointer-gestures-unstable-v1.xml \
		xdg-shell/xdg-shell-unstable-v6.xml \
		xdg-shell/xdg-shell-unstable-v5.xml \
		xdg-foreign/xdg-foreign-unstable-v2.xml \
		xdg-foreign/xdg-foreign-unstable-v1.xml \
		keyboard-shortcuts-inhibit/keyboard-shortcuts-inhibit-unstable-v1.xml \
		input-timestamps/input-timestamps-unstable-v1.xml \
		pointer-constraints/pointer-constraints-unstable-v1.xml \
		relative-pointer/relative-pointer-unstable-v1.xml \
		primary-selection/primary-selection-unstable-v1.xml \
		xdg-output/xdg-output-unstable-v1.xml \
		linux-explicit-synchronization/linux-explicit-synchronization-unstable-v1.xml \
		input-method/input-method-unstable-v1.xml \
		xdg-decoration/xdg-decoration-unstable-v1.xml \
		fullscreen-shell/fullscreen-shell-unstable-v1.xml \
		text-input/text-input-unstable-v1.xml \
		text-input/text-input-unstable-v3.xml \
                tablet/tablet-unstable-v2.xml

DATADIR=        ${OBJTOP}/tmp${PREFIX}/share/wayland-protocols

all: install
build: install
obj:
includes: 

install:
.for group in stable staging unstable
.for file in ${${group:tu}}
	install -d -m 0755 -g 0 -o 0 ${DATADIR}/${group}/$$(dirname ${file})
	install -m 0644 -g 0 -o 0 ${.CURDIR}/${group}/${file} \
                ${DATADIR}/${group}/$$(dirname ${file})
.endfor
.endfor

clean:
	rm -rf ${DATADIR}

