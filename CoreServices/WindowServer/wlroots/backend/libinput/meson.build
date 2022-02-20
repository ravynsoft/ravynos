msg = ['Required for libinput backend support.']
if 'libinput' in backends
	msg += 'Install "libinput" or disable the libinput backend.'
endif

libinput = dependency(
	'libinput',
	version: '>=1.14.0',
	required: 'libinput' in backends,
	not_found_message: '\n'.join(msg),
)

if not libinput.found()
	subdir_done()
endif

wlr_files += files(
	'backend.c',
	'events.c',
	'keyboard.c',
	'pointer.c',
	'switch.c',
	'tablet_pad.c',
	'tablet_tool.c',
	'touch.c',
)

features += { 'libinput-backend': true }
wlr_deps += libinput

# libinput hold gestures are available since 1.19.0
add_project_arguments(
	'-DLIBINPUT_HAS_HOLD_GESTURES=@0@'.format(libinput.version().version_compare('>=1.19.0').to_int()),
	language: 'c',
)
