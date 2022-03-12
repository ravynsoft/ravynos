cfdata = configuration_data()
cfdata.set('libexecdir', get_option('prefix') / get_option('libexecdir'))
cfdata.set('localedir', get_option('prefix') / get_option('localedir'))
cfdata.set('sysconfdir', get_option('prefix') / get_option('sysconfdir'))
cfdata.set('package', meson.project_name().to_lower())

configure_file(
  configuration: cfdata,
  input: 'waybox.sh',
  output: 'waybox',
  install: true,
  install_dir: get_option('prefix') / get_option('bindir'),
  install_mode: 'rwxr-xr-x',
)

scripts = files(
  'autostart',
  'environment',
  'xdg-autostart',
  )

wb_xdg_dir = get_option('prefix') / get_option('sysconfdir') + '/xdg/waybox'

install_data(
  scripts,
  install_dir: wb_xdg_dir,
  install_mode: 'rwxr-xr-x',
  )

install_data(
  'rc.xml',
  install_dir: wb_xdg_dir,
  install_mode: 'rw-r--r--',
  )

install_data(
  'waybox.desktop',
  install_dir: get_option('prefix') / get_option('datadir') + '/wayland-sessions',
  install_mode: 'rw-r--r--',
  )
