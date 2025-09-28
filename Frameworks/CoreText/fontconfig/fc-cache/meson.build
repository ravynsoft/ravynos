fccache = executable('fc-cache', ['fc-cache.c', fcstdint_h, alias_headers, ft_alias_headers],
  include_directories: [incbase, incsrc],
  link_with: [libfontconfig],
  c_args: c_args,
  install: true,
)

tools_man_pages += ['fc-cache']

# Do not try to execute target's fc-cache on host when cross compiling
if get_option('cache-build').enabled() and not meson.is_cross_build()
  meson.add_install_script(fccache, '-s', '-f', '-v',
    skip_if_destdir: true)
endif
