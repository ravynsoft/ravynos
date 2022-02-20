project(
	'wlroots',
	'c',
	version: '0.16.0',
	license: 'MIT',
	meson_version: '>=0.58.1',
	default_options: [
		'c_std=c11',
		'warning_level=2',
		'werror=true',
	],
)

# When doing a major or minor release, *always* increase soversion. This isn't
# necessary for bugfix releases. Increasing soversion is required because
# wlroots never guarantees ABI stability -- only API stability is guaranteed
# between minor releases.
soversion = 11

little_endian = host_machine.endian() == 'little'
big_endian = host_machine.endian() == 'big'

add_project_arguments([
	'-DWLR_USE_UNSTABLE',
	'-DWLR_LITTLE_ENDIAN=@0@'.format(little_endian.to_int()),
	'-DWLR_BIG_ENDIAN=@0@'.format(big_endian.to_int()),
], language: 'c')

cc = meson.get_compiler('c')

add_project_arguments(cc.get_supported_arguments([
	'-Wundef',
	'-Wlogical-op',
	'-Wmissing-include-dirs',
	'-Wold-style-definition',
	'-Wpointer-arith',
	'-Winit-self',
	'-Wstrict-prototypes',
	'-Wimplicit-fallthrough=2',
	'-Wendif-labels',
	'-Wstrict-aliasing=2',
	'-Woverflow',
	'-Wmissing-prototypes',
	'-Walloca',

	'-Wno-missing-braces',
	'-Wno-missing-field-initializers',
	'-Wno-unused-parameter',
]), language: 'c')

# Compute the relative path used by compiler invocations.
source_root = meson.current_source_dir().split('/')
build_root = meson.global_build_root().split('/')
relative_dir_parts = []
i = 0
in_prefix = true
foreach p : build_root
	if i >= source_root.length() or not in_prefix or p != source_root[i]
		in_prefix = false
		relative_dir_parts += '..'
	endif
	i += 1
endforeach
i = 0
in_prefix = true
foreach p : source_root
	if i >= build_root.length() or not in_prefix or build_root[i] != p
		in_prefix = false
		relative_dir_parts += p
	endif
	i += 1
endforeach
relative_dir = join_paths(relative_dir_parts) + '/'

# Strip relative path prefixes from the code if possible, otherwise hide them.
if cc.has_argument('-fmacro-prefix-map=/prefix/to/hide=')
	add_project_arguments(
		'-fmacro-prefix-map=@0@='.format(relative_dir),
		language: 'c',
	)
else
	add_project_arguments(
		'-DWLR_REL_SRC_DIR="@0@"'.format(relative_dir),
		language: 'c',
	)
endif

features = {
	'drm-backend': false,
	'x11-backend': false,
	'libinput-backend': false,
	'xwayland': false,
	'gles2-renderer': false,
	'vulkan-renderer': false,
}
internal_features = {
	'xcb-errors': false,
}

wayland_project_options = ['tests=false', 'documentation=false']
wayland_server = dependency('wayland-server',
	version: '>=1.20',
	fallback: 'wayland',
	default_options: wayland_project_options,
)

drm = dependency('libdrm',
	version: '>=2.4.109',
	fallback: 'libdrm',
	default_options: [
		'libkms=false',
		'intel=false',
		'radeon=false',
		'amdgpu=false',
		'nouveau=false',
		'vmwgfx=false',
		'omap=false',
		'exynos=false',
		'freedreno=false',
		'tegra=false',
		'vc4=false',
		'etnaviv=false',
		'cairo-tests=false',
		'man-pages=false',
		'valgrind=false',
	],
)
gbm = dependency('gbm', version: '>=17.1.0')
xkbcommon = dependency('xkbcommon')
udev = dependency('libudev')
pixman = dependency('pixman-1')
math = cc.find_library('m')
rt = cc.find_library('rt')

wlr_files = []
wlr_deps = [
	wayland_server,
	drm,
	gbm,
	xkbcommon,
	udev,
	pixman,
	math,
	rt,
]

subdir('protocol')
subdir('render')

subdir('backend')
subdir('types')
subdir('util')
subdir('xcursor')
subdir('xwayland')

subdir('include')

foreach name, have : internal_features
	add_project_arguments(
		'-DHAS_@0@=@1@'.format(name.underscorify().to_upper(), have.to_int()),
		language: 'c',
	)
endforeach

wlr_inc = include_directories('include')
proto_inc = include_directories('protocol')

symbols_file = 'wlroots.syms'
symbols_flag = '-Wl,--version-script,@0@/@1@'.format(meson.current_source_dir(), symbols_file)
lib_wlr = library(
	meson.project_name(), wlr_files,
	soversion: soversion,
	dependencies: wlr_deps,
	include_directories: [wlr_inc, proto_inc],
	install: true,
	link_args: symbols_flag,
	link_depends: symbols_file,
)

wlr_vars = {}
foreach name, have : features
	wlr_vars += { 'have_' + name.underscorify(): have.to_string() }
endforeach

wlroots = declare_dependency(
	link_with: lib_wlr,
	dependencies: wlr_deps,
	include_directories: wlr_inc,
	variables: wlr_vars,
)

meson.override_dependency('wlroots', wlroots)

summary(features + internal_features, bool_yn: true)

if get_option('examples')
	subdir('examples')
	subdir('tinywl')
endif

pkgconfig = import('pkgconfig')
pkgconfig.generate(lib_wlr,
	version: meson.project_version(),
	filebase: meson.project_name(),
	name: meson.project_name(),
	description: 'Wayland compositor library',
	variables: wlr_vars,
)
