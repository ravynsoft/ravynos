vulkan_shaders_src = [
	'common.vert',
	'texture.frag',
	'quad.frag',
]

vulkan_shaders = []
foreach shader : vulkan_shaders_src
	name = shader.underscorify() + '_data'
	args = [glslang, '-V', '@INPUT@', '-o', '@OUTPUT@', '--vn', name]
	if glslang_version.version_compare('>=11.0.0')
		args += '--quiet'
	endif
	header = custom_target(
		shader + '_spv',
		output: shader + '.h',
		input: shader,
		command: args)

	vulkan_shaders += [header]
endforeach

wlr_files += vulkan_shaders
