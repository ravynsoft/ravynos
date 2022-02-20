x11_libs = []
x11_required = [
	'xcb',
	'xcb-dri3',
	'xcb-present',
	'xcb-render',
	'xcb-renderutil',
	'xcb-shm',
	'xcb-xfixes',
	'xcb-xinput',
]

msg = ['Required for X11 backend support.']
if 'x11' in backends
	msg += 'Install "@0@" or disable the X11 backend.'
endif

foreach lib : x11_required
	dep = dependency(lib,
		required: 'x11' in backends,
		not_found_message: '\n'.join(msg).format(lib),
	)
	if not dep.found()
		subdir_done()
	endif

	x11_libs += dep
endforeach

wlr_files += files(
	'backend.c',
	'input_device.c',
	'output.c',
)
wlr_deps += x11_libs
features += { 'x11-backend': true }
