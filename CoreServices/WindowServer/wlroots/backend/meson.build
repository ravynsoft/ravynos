wlr_files += files('backend.c')

all_backends = ['drm', 'libinput', 'x11']
backends = get_option('backends')
if 'auto' in backends and get_option('auto_features').enabled()
	backends = all_backends
elif 'auto' in backends and get_option('auto_features').disabled()
	backends = []
endif

foreach backend : all_backends
	if backend in backends or 'auto' in backends
		subdir(backend)
	endif
endforeach

subdir('multi')
subdir('wayland')
subdir('headless')

subdir('session')
