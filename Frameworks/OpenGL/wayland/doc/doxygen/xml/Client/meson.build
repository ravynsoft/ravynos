tgt = custom_target(
	'xml-Client-doc',
	command: [
		gen_doxygen,
		# XXX pass doxygen path as argument
		'--builddir=@OUTDIR@',
		'--section=Client',
		'--output-format=xml',
		wayland_doxygen,
		'@INPUT@',
	],
	input: [ shared_files, client_files ],
	output: [ 'combine.xslt', 'index.xml' ],
	depends: [dot_png, dot_map]
)

doxygen_Client_combine_xslt = tgt[0]
doxygen_Client_index_xml = tgt[1]
