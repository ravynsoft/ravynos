if not get_option('libraries')
	error('-Ddocumentation=true requires -Dlibraries=true')
endif

dot = find_program('dot')
doxygen = find_program('doxygen')
xsltproc = find_program('xsltproc')
xmlto = find_program('xmlto')

cmd = run_command(doxygen, '--version', check: true)
message('doxygen: ' + cmd.stdout().strip())
vers = cmd.stdout().strip()
if vers.version_compare('< 1.6.0')
	error('Doxygen 1.6 or later is required for building documentation, found @0@.'.format(vers))
endif

cmd = run_command(dot, '-V', check: true)
message('dot: ' + cmd.stderr().strip())
vers = cmd.stderr().split('version')[1].strip().split(' ')[0]
if vers.version_compare('< 2.26.0')
	error('Dot (Graphviz) 2.26 or later is required for building documentation, found @0@.'.format(vers))
endif

manpage_xsl = 'http://docbook.sourceforge.net/release/xsl/current/manpages/docbook.xsl'
cmd = run_command(xsltproc, '--nonet', manpage_xsl, check: false)
if cmd.returncode() != 0
	error('The style sheet for man pages providing "@0@" was not found.'.format(manpage_xsl))
endif

publican_install_prefix = join_paths(
	get_option('prefix'),
	get_option('datadir'),
	'doc',
	meson.project_name(),
	'Wayland', 'en-US'
)

publican_html_dir = 'html'

subdir('doxygen')
subdir('publican')
