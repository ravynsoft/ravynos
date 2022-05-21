labwc_sources = files(
  'action.c',
  'cursor.c',
  'damage.c',
  'desktop.c',
  'foreign.c',
  'interactive.c',
  'keyboard.c',
  'key-state.c',
  'layers.c',
  'main.c',
  'osd.c',
  'output.c',
  'resistance.c',
  'seat.c',
  'server.c',
  'ssd.c',
  'subsurface.c',
  'theme.c',
  'view.c',
  'view-child.c',
  'view-impl.c',
  'xdg.c',
  'xdg-deco.c',
  'xdg-popup.c',
)

if have_xwayland
  labwc_sources += files(
    'xwayland.c',
    'xwayland-unmanaged.c',
  )
endif


subdir('common')
subdir('config')
subdir('xbm')
subdir('menu')
