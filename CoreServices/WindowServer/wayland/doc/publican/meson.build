merge_mapcoords_xsl = files('merge-mapcoords.xsl')

subdir('sources')

custom_target(
	'Wayland-docbook-html',
	command: [
		xmlto,
		'--skip-validation',
		'--stringparam', 'chunker.output.encoding=UTF-8',
		'--stringparam', 'chunk.section.depth=0',
		'--stringparam', 'toc.section.depth=1',
		'--stringparam', 'generate.consistent.ids=1',
		'--stringparam', 'html.stylesheet=css/default.css',
		'-o', '@OUTPUT@',
		'html',
		'@INPUT@'
	],
	input: publican_processed_main,
	output: publican_html_dir,
	depend_files: publican_copied_sources,
	depends: [
		publican_processed_targets,
		ClientAPI_xml,
		ServerAPI_xml,
		ProtocolSpec_xml,
		ProtocolInterfaces_xml
	],
	build_by_default: true,
	install: true,
	install_dir: publican_install_prefix
)
