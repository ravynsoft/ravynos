# Sphinx build
sphinx = find_program('sphinx-build-3', 'sphinx-build', required : false)
if not sphinx.found()
	error('Program "sphinx-build" not found or not executable. Try building with -Ddocumentation=false')
endif

yq = find_program('yq', required : false)
if not yq.found()
	warning('Program "yq" not found or not executable. Dependency list will not be built.')
endif

sphinx_config = configuration_data()
sphinx_config.set('PROJECT_NAME', meson.project_name())
sphinx_config.set('PROJECT_VERSION', meson.project_version())
sphinx_config.set('BUILDDIR', meson.current_build_dir())
sphinx_config.set('HTTP_DOC_LINK', doc_url)

git_version_page = vcs_tag(command : ['git', 'log', '-1', '--format=%H'],
			   fallback : 'unknown',
			   input : 'git_version.py.in',
			   output : 'git_version.py',
			   replace_string: '__GIT_VERSION__')

sphinx_conf_py = configure_file(input : 'conf.py.in',
				output : 'conf.py',
				configuration : sphinx_config)

# 404 replacements for old URLs
# The switch to sphinx caused a few pages to be renamed, sphinx uses
# filename.html whereas doxygen used whatever the @page foo was. So old docs
# *mostly* used underscores, now we're consistent with dashes.
# We can't use htaccess on the server, so let's auto-generate a 404 list
# with a basic page telling users that the link has moved. This can be
# removed in a few months, towards the end of 2018.
#
# File list is: [current-sphinx-input-file, old-generated-page]
# If they're the same they'll be ignored.
src_404s = [
	[ 'absolute-axes.rst', 'absolute_axes.html'],
	[ 'absolute-coordinate-ranges.rst', 'absolute_coordinate_ranges.html'],
	[ 'architecture.rst', 'architecture.html'],
	[ 'building.rst', 'building_libinput.html'],
	[ 'button-debouncing.rst', 'button_debouncing.html'],
	[ 'clickpad-softbuttons.rst', 'clickpad_softbuttons.html'],
	[ 'configuration.rst', 'config_options.html'],
	[ 'contributing.rst', 'contributing.html'],
	[ 'development.rst', 'development.html'],
	[ 'device-configuration-via-udev.rst', 'udev_config.html'],
	[ 'device-quirks.rst', 'device-quirks.html'],
	[ 'faqs.rst', 'faq.html'],
	[ 'features.rst', 'features.html'],
	[ 'gestures.rst', 'gestures.html'],
	[ 'incorrectly-enabled-hires.rst', 'incorrectly-enabled-hires.html'],
	[ 'middle-button-emulation.rst', 'middle_button_emulation.html'],
	[ 'normalization-of-relative-motion.rst', 'motion_normalization.html'],
	[ 'palm-detection.rst', 'palm_detection.html'],
	[ 'pointer-acceleration.rst', 'pointer-acceleration.html'],
	[ 'reporting-bugs.rst', 'reporting_bugs.html'],
	[ 'scrolling.rst', 'scrolling.html'],
	[ 'seats.rst', 'seats.html'],
	[ 'switches.rst', 'switches.html'],
	[ 't440-support.rst', 't440_support.html'],
	[ 'tablet-support.rst', 'tablet-support.html'],
	[ 'tapping.rst', 'tapping.html'],
	[ 'test-suite.rst', 'test-suite.html'],
	[ 'timestamps.rst', 'timestamps.html'],
	[ 'tools.rst', 'tools.html'],
	[ 'touchpad-jitter.rst', 'touchpad_jitter.html'],
	[ 'touchpad-jumping-cursors.rst', 'touchpad_jumping_cursor.html'],
	[ 'touchpad-pressure.rst', 'touchpad_pressure.html'],
	[ 'touchpads.rst', 'touchpads.html'],
	[ 'trackpoints.rst', 'trackpoints.html'],
	[ 'troubleshooting.rst', 'troubleshooting.html'],
	[ 'what-is-libinput.rst', 'what_is_libinput.html'],
]

dst_404s = []
foreach s404 : src_404s
	target = s404[0]
	oldpage = s404[1]
	tname = target.split('.rst')[0]
	oname = oldpage.split('.html')[0]

	if tname != oname
		config_404 = configuration_data()
		config_404.set('TARGET', '@0@.html'.format(tname))
		c = configure_file(input : '404.rst',
				   output : '@0@.rst'.format(oname),
				   configuration : config_404)
		dst_404s += [c]
	endif
endforeach

src_rst = files(
	# dot drawings
	'dot/seats-sketch.gv',
	'dot/seats-sketch-libinput.gv',
	'dot/libinput-stack-wayland.gv',
	'dot/libinput-stack-xorg.gv',
	'dot/libinput-stack-gnome.gv',
	'dot/evemu.gv',
	'dot/libinput-record.gv',
	# svgs
	'svg/button-debouncing-wave-diagram.svg',
	'svg/button-scrolling.svg',
	'svg/clickfinger.svg',
	'svg/clickfinger-distance.svg',
	'svg/edge-scrolling.svg',
	'svg/gesture-2fg-ambiguity.svg',
	'svg/palm-detection.svg',
	'svg/pinch-gestures.svg',
	'svg/pinch-gestures-softbuttons.svg',
	'svg/ptraccel-custom.svg',
	'svg/ptraccel-linear.svg',
	'svg/ptraccel-low-dpi.svg',
	'svg/ptraccel-touchpad.svg',
	'svg/ptraccel-trackpoint.svg',
	'svg/software-buttons.svg',
	'svg/software-buttons-conditions.svg',
	'svg/software-buttons-thumbpress.svg',
	'svg/software-buttons-visualized.svg',
	'svg/swipe-gestures.svg',
	'svg/tablet-axes.svg',
	'svg/tablet-cintiq24hd-modes.svg',
	'svg/tablet-interfaces.svg',
	'svg/tablet-intuos-modes.svg',
	'svg/tablet-left-handed.svg',
	'svg/tablet-out-of-bounds.svg',
	'svg/tablet.svg',
	'svg/tap-n-drag.svg',
	'svg/thumb-detection.svg',
	'svg/top-software-buttons.svg',
	'svg/touchscreen-gestures.svg',
	'svg/trackpoint-delta-illustration.svg',
	'svg/twofinger-scrolling.svg',
	# rst files
	'absolute-axes.rst',
	'absolute-coordinate-ranges.rst',
	'architecture.rst',
	'building.rst',
	'button-debouncing.rst',
	'clickpad-softbuttons.rst',
	'clickpad-with-right-button.rst',
	'contributing.rst',
	'device-configuration-via-udev.rst',
	'device-quirks.rst',
	'faqs.rst',
	'gestures.rst',
	'incorrectly-enabled-hires.rst',
	'ignoring-devices.rst',
	'middle-button-emulation.rst',
	'normalization-of-relative-motion.rst',
	'palm-detection.rst',
	'pointer-acceleration.rst',
	'reporting-bugs.rst',
	'scrolling.rst',
	'seats.rst',
	'switches.rst',
	't440-support.rst',
	'tablet-support.rst',
	'tapping.rst',
	'test-suite.rst',
	'timestamps.rst',
	'tablet-debugging.rst',
	'tools.rst',
	'touchpad-jumping-cursors.rst',
	'touchpad-pressure.rst',
	'touchpad-pressure-debugging.rst',
	'touchpad-jitter.rst',
	'touchpad-thumb-detection.rst',
	'touchpads.rst',
	'trackpoints.rst',
	'trackpoint-configuration.rst',
	'what-is-libinput.rst',
	'wheel-api.rst',
	'features.rst',
	'development.rst',
	'troubleshooting.rst',
	'configuration.rst',
)

src_sphinx = []
foreach f : src_rst
	sf = configure_file(input: f,
			    output: '@PLAINNAME@',
			    copy : true)
	src_sphinx += [ sf ]
endforeach

configure_file(input: 'index.rst',
	       output: 'index.rst',
	       configuration: sphinx_config)

dependencies_config = configuration_data()
if yq.found()
	distributions = ['fedora', 'ubuntu', 'debian', 'arch', 'alpine']
	foreach distro : distributions
		yq_filter = '.distributions[] | select(.name == "@0@") | .packages | join(" ")'.format(distro)
		deps = run_command(yq, '-r', yq_filter,
				   dir_gitlab_ci / 'config.yml',
				   check: true).stdout()
		dependencies_config.set('@0@_PACKAGES'.format(distro.to_upper()), deps)
endforeach
endif
configure_file(input: 'dependencies.rst',
	       output: 'dependencies.rst',
	       configuration: dependencies_config)

# do not use -j, it breaks on Ubuntu
sphinx_output_dir = 'Documentation'
custom_target('sphinx',
	      input : [ sphinx_conf_py, git_version_page ] + src_sphinx + dst_404s,
	      output : [ sphinx_output_dir ],
	      command : [ sphinx, '-q', '-b', 'html',
			  '-d', meson.current_build_dir() / 'doctrees',
			  meson.current_build_dir(), sphinx_output_dir],
	      build_by_default : true)
