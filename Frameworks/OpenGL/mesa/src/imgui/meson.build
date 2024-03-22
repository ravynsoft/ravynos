libimgui_core = static_library(
  'imgui_core',
  files('imgui.cpp', 'imgui_draw.cpp', 'imgui_widgets.cpp'),
  cpp_args : ['-w'],
  install : false
)

libimgui_core_dep = declare_dependency(
  link_with : libimgui_core,
  include_directories : include_directories('.')
)
