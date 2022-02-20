icondir = get_option('icon_directory')
if icondir == ''
	icondir = join_paths(get_option('prefix'), get_option('datadir'), 'icons')
endif

add_project_arguments('-DICONDIR="@0@"'.format(icondir), language : 'c')

wlr_files += files(
	'wlr_xcursor.c',
	'xcursor.c',
)
