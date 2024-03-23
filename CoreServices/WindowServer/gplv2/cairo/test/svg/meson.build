if librsvg_dep.found()
  executable('svg-render',
             'svg-render.c',
             dependencies: [libcairo_dep, librsvg_dep])
endif

if cc.links(files(meson.project_source_root() / 'meson-cc-tests/fuzzer.c'), args: '-fsanitize=fuzzer,address')
  subdir('fuzzer')
endif
