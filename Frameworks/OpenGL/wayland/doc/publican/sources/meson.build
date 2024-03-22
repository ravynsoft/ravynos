ProtocolSpec_xml = custom_target(
	'ProtocolSpec.xml',
	command: [ xsltproc, '-o', '@OUTPUT@', files('../protocol-to-docbook.xsl'), '@INPUT@' ],
	input: wayland_protocol_xml,
	output: 'ProtocolSpec.xml'
)

ProtocolInterfaces_xml = custom_target(
	'ProtocolInterfaces.xml',
	command: [ xsltproc, '-o', '@OUTPUT@', files('../protocol-interfaces-to-docbook.xsl'), '@INPUT@' ],
	input: wayland_protocol_xml,
	output: 'ProtocolInterfaces.xml'
)

ClientAPI_combined = custom_target(
	'ClientAPI-combined',
	command: [ xsltproc, '-o', '@OUTPUT@', '@INPUT@' ],
	input: [ doxygen_Client_combine_xslt, doxygen_Client_index_xml ],
	output: 'ClientAPI-combined.xml'
)

to_publican_xsl = files('../doxygen-to-publican.xsl')

ClientAPI_xml = custom_target(
	'ClientAPI.xml',
	command: [ xsltproc, '-o', '@OUTPUT@', '--stringparam', 'which', 'Client', to_publican_xsl, '@INPUT@' ],
	input: ClientAPI_combined,
	output: 'ClientAPI.xml'
)

ServerAPI_combined = custom_target(
	'ServerAPI-combined',
	command: [ xsltproc, '-o', '@OUTPUT@', '@INPUT@' ],
	input: [ doxygen_Server_combine_xslt, doxygen_Server_index_xml ],
	output: 'ServerAPI-combined.xml'
)

ServerAPI_xml = custom_target(
	'ServerAPI.xml',
	command: [ xsltproc, '-o', '@OUTPUT@', '--stringparam', 'which', 'Server', to_publican_xsl, '@INPUT@' ],
	input: ServerAPI_combined,
	output: 'ServerAPI.xml'
)


publican_sources = [
	'Wayland.ent',
	# 'Wayland.xml', # handled specially
	'Book_Info.xml',
	'Author_Group.xml',
	'Foreword.xml',
	'Preface.xml',
	'Revision_History.xml',
	'Protocol.xml',
	'Xwayland.xml',
	'Compositors.xml',
	'Client.xml',
	'Server.xml'
]

publican_processed_main = configure_file(
	input: 'Wayland.xml',
	output: 'Wayland.xml',
	copy: true
)

publican_copied_sources = []
foreach src: publican_sources
	publican_copied_sources += configure_file(
		input: src,
		output: src,
		copy: true
	)
endforeach

publican_processed_sources = [
	'Architecture.xml',
	'Introduction.xml'
]

publican_processed_targets = []
foreach src: publican_processed_sources
	publican_processed_targets += custom_target(
		src,
		command: [ xsltproc, '-o', '@OUTPUT@', '--stringparam', 'basedir', '.', merge_mapcoords_xsl, '@INPUT@' ],
		input: src,
		output: src
	)
endforeach

publican_css_sources = files([
	'css/brand.css',
	'css/common.css',
	'css/default.css',
	'css/epub.css',
	'css/print.css'
])

install_data(
	publican_css_sources,
	install_dir: join_paths(publican_install_prefix, publican_html_dir, 'css')
)

publican_img_sources = files([
	'images/icon.svg',
	'images/wayland.png',
	'images/xwayland-architecture.png'
])

install_data(
	publican_img_sources,
	install_dir: join_paths(publican_install_prefix, publican_html_dir, 'images')
)
