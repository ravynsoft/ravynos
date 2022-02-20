glesv2 = dependency('glesv2', required: 'gles2' in renderers)

if not (glesv2.found() and egl.found())
	subdir_done()
endif

features += { 'gles2-renderer': true }
wlr_deps += glesv2

wlr_files += files(
	'pixel_format.c',
	'renderer.c',
	'shaders.c',
	'texture.c',
)
