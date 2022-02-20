have_listenfd = false
have_no_touch_pointer_emulation = false
if xwayland.found()
	xwayland_path = xwayland.get_variable('xwayland')
	have_listenfd = xwayland.get_variable('have_listenfd',
		default_value: 'false') == 'true'
	have_no_touch_pointer_emulation = xwayland.get_variable(
		'have_no_touch_pointer_emulation', default_value: 'false') == 'true'
else
	xwayland_path = xwayland_prog.full_path()
endif

xwayland_config_data = configuration_data()
xwayland_config_data.set_quoted('XWAYLAND_PATH', xwayland_path)
xwayland_config_data.set10('HAVE_XWAYLAND_LISTENFD', have_listenfd)
xwayland_config_data.set10('HAVE_XWAYLAND_NO_TOUCH_POINTER_EMULATION', have_no_touch_pointer_emulation)
configure_file(
	output: 'config.h',
	configuration: xwayland_config_data,
)
