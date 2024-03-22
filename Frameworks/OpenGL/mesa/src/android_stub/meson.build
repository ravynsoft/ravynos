if with_android_stub
  stub_libs = []
  lib_names = ['cutils', 'hardware', 'log', 'nativewindow', 'sync']

  if with_libbacktrace
    lib_names += ['backtrace']
  endif

  foreach lib : lib_names
    stub_libs += shared_library(
      lib,
      files(lib + '_stub.cpp'),
      include_directories : inc_include,
      install : false,
    )
  endforeach

  dep_android = declare_dependency(
    link_with : stub_libs,
  )
endif
