msg = []
if 'vulkan' in renderers
	msg += 'Install "@0@" or pass "-Dvulkan=disabled" to disable it.'
else
	msg += 'Required for vulkan renderer support.'
endif

dep_vulkan = dependency('vulkan',
	version: '>=1.2.182',
	required: 'vulkan' in renderers,
	not_found_message: '\n'.join(msg).format('vulkan')
)
if not dep_vulkan.found()
	subdir_done()
endif

# Vulkan headers are installed separately from the loader (which ships the
# pkg-config file)
if not cc.check_header('vulkan/vulkan.h', dependencies: dep_vulkan)
	if 'vulkan' in renderers
		error('\n'.join(msg).format('vulkan-headers'))
	else
		subdir_done()
	endif
endif

glslang = find_program('glslangValidator', native: true, required: false)
if not glslang.found()
	if 'vulkan' in renderers
		error('\n'.join(msg).format('glslang'))
	else
		subdir_done()
	endif
endif

glslang_version_info = run_command(glslang, '--version', check: true).stdout()
glslang_version = glslang_version_info.split('\n')[0].split(':')[-1]

wlr_files += files(
	'renderer.c',
	'texture.c',
	'vulkan.c',
	'util.c',
	'pixel_format.c',
)

wlr_deps += dep_vulkan
features += { 'vulkan-renderer': true }

subdir('shaders')
