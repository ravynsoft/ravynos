conf_files = [
  '05-reset-dirs-sample.conf',
  '09-autohint-if-no-hinting.conf',
  '10-autohint.conf',
  '10-hinting-full.conf',
  '10-hinting-medium.conf',
  '10-hinting-none.conf',
  '10-hinting-slight.conf',
  '10-no-antialias.conf',
  '10-scale-bitmap-fonts.conf',
  '10-sub-pixel-bgr.conf',
  '10-sub-pixel-none.conf',
  '10-sub-pixel-rgb.conf',
  '10-sub-pixel-vbgr.conf',
  '10-sub-pixel-vrgb.conf',
  '10-unhinted.conf',
  '10-yes-antialias.conf',
  '11-lcdfilter-default.conf',
  '11-lcdfilter-legacy.conf',
  '11-lcdfilter-light.conf',
  '20-unhint-small-vera.conf',
  '25-unhint-nonlatin.conf',
  '30-metric-aliases.conf',
  '40-nonlatin.conf',
  '45-generic.conf',
  '45-latin.conf',
  '48-spacing.conf',
  '49-sansserif.conf',
  '50-user.conf',
  '51-local.conf',
  '60-generic.conf',
  '60-latin.conf',
  '65-fonts-persian.conf',
  '65-khmer.conf',
  '65-nonlatin.conf',
  '69-unifont.conf',
  '70-no-bitmaps.conf',
  '70-yes-bitmaps.conf',
  '80-delicious.conf',
  '90-synthetic.conf',
]

preferred_hinting = get_option('default-hinting')
preferred_sub_pixel_rendering = get_option('default-sub-pixel-rendering')

conf_links = [
  '10-hinting-@0@.conf'.format(preferred_hinting),
  '10-scale-bitmap-fonts.conf',
  '10-sub-pixel-@0@.conf'.format(preferred_sub_pixel_rendering),
  '10-yes-antialias.conf',
  '11-lcdfilter-default.conf',
  '20-unhint-small-vera.conf',
  '30-metric-aliases.conf',
  '40-nonlatin.conf',
  '45-generic.conf',
  '45-latin.conf',
  '48-spacing.conf',
  '49-sansserif.conf',
  '50-user.conf',
  '51-local.conf',
  '60-generic.conf',
  '60-latin.conf',
  '65-fonts-persian.conf',
  '65-nonlatin.conf',
  '69-unifont.conf',
  '80-delicious.conf',
  '90-synthetic.conf',
]

install_data(conf_files, install_dir: fc_templatedir)

meson.add_install_script('link_confs.py', fc_templatedir,
  fc_configdir,
  conf_links,
)

# 35-lang-normalize.conf
orths = []
foreach o : orth_files          # orth_files is from fc-lang/meson.build
  o = o.split('.')[0]           # strip filename suffix
  if not o.contains('_')        # ignore those with an underscore
    orths += [o]
  endif
endforeach

custom_target('35-lang-normalize.conf',
  output: '35-lang-normalize.conf',
  command: [find_program('write-35-lang-normalize-conf.py'), ','.join(orths), '@OUTPUT@'],
  install_dir: fc_templatedir,
  install: true)

# README
readme_cdata = configuration_data()
readme_cdata.set('TEMPLATEDIR', fc_templatedir)
configure_file(output: 'README',
  input: 'README.in',
  configuration: readme_cdata,
  install_dir: fc_configdir,
  install: true)
