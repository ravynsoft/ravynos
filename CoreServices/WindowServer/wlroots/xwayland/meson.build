xwayland_libs = []
xwayland_required = [
	'xcb',
	'xcb-composite',
	'xcb-icccm',
	'xcb-render',
	'xcb-res',
	'xcb-xfixes',
]
xwayland_optional = {
	'xcb-errors': 'Required for printing X11 errors.',
}

msg = []
if get_option('xwayland').enabled()
	msg += 'Install "@0@" or pass "-Dxwayland=disabled".'
endif
if not get_option('xwayland').disabled()
	msg += 'Required for Xwayland support.'
endif

xwayland = dependency('xwayland', required: false)
if not xwayland.found()
	# There's no Xwayland release with the pkg-config file shipped yet.
	xwayland_prog = find_program('Xwayland', required: false)
	if not xwayland_prog.found()
		if get_option('xwayland').enabled()
			error('\n'.join(msg).format('xwayland'))
		else
			subdir_done()
		endif
	endif
endif

foreach lib : xwayland_required
	dep = dependency(lib,
		required: get_option('xwayland'),
		not_found_message: '\n'.join(msg).format(lib),
	)
	if not dep.found()
		subdir_done()
	endif

	xwayland_libs += dep
endforeach

foreach lib, desc : xwayland_optional
	msg = []
	if get_option(lib).enabled()
		msg += 'Install "@0@" or pass "-D@0@=disabled".'
	endif
	if not get_option(lib).disabled()
		msg += desc
	endif

	dep = dependency(lib,
		required: get_option(lib),
		not_found_message: '\n'.join(msg).format(lib),
	)

	internal_features += { lib: dep.found() }
	xwayland_libs += dep
endforeach

wlr_files += files(
	'selection/dnd.c',
	'selection/incoming.c',
	'selection/outgoing.c',
	'selection/selection.c',
	'server.c',
	'sockets.c',
	'xwayland.c',
	'xwm.c',
)
wlr_deps += xwayland_libs
features += { 'xwayland': true }
