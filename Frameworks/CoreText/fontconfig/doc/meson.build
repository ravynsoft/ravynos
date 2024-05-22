doc_targets = []

docbook2man = find_program('docbook2man', required: get_option('doc-man'))
docbook2txt = find_program('docbook2txt', required: get_option('doc-txt'))
docbook2pdf = find_program('docbook2pdf', required: get_option('doc-pdf'))
docbook2html = find_program('docbook2html', required: get_option('doc-html'))

# docbook is very spammy
run_quiet = find_program('run-quiet.py')

# .fncs files
doc_funcs_fncs = [
  'fcatomic',
  'fcblanks',
  'fccache',
  'fccharset',
  'fcconfig',
  'fcconstant',
  'fcdircache',
  'fcfile',
  'fcfontset',
  'fcformat',
  'fcfreetype',
  'fcinit',
  'fclangset',
  'fcmatrix',
  'fcobjectset',
  'fcobjecttype',
  'fcpattern',
  'fcrange',
  'fcstring',
  'fcstrset',
  'fcvalue',
  'fcweight',
]

fncs_files = []
foreach f : doc_funcs_fncs
  fncs_files += files('@0@.fncs'.format(f))
endforeach

man_pages = []

extract_man_list = find_program('extract-man-list.py')
man_list = run_command(extract_man_list, fncs_files, check: true).stdout().split()

foreach m : man_list
  man_pages += ['@0@.3'.format(m)]
endforeach

# Generate sgml pages for funcs
edit_sgml = find_program('edit-sgml.py')

# copy into build directory, it includes generated files from build directory
fontconfig_devel_sgml = configure_file(output: 'fontconfig-devel.sgml',
  input: 'fontconfig-devel.sgml',
  copy: true)

fontconfig_user_sgml = configure_file(output: 'fontconfig-user.sgml',
  input: 'fontconfig-user.sgml',
  copy: true)

version_conf = configuration_data()
version_conf.set('VERSION', meson.project_version())

configure_file(output: 'version.sgml',
  input: 'version.sgml.in',
  configuration: version_conf)

confdir_conf = configuration_data()
confdir_conf.set('BASECONFIGDIR', fc_configdir)

confdir_sgml = configure_file(output: 'confdir.sgml',
  input: 'confdir.sgml.in',
  configuration: confdir_conf)

funcs_sgml = []

foreach f : doc_funcs_fncs
  funcs_sgml += [custom_target('@0@.sgml'.format(f),
    input: [files('func.sgml'), files('@0@.fncs'.format(f))],
    output: '@0@.sgml'.format(f),
    command: [edit_sgml, '@INPUT0@', '@INPUT1@', '@OUTPUT@'],
    install: false)]
endforeach

if docbook2man.found()
  doc_targets += ['man']

  custom_target('devel-man',
    input: [fontconfig_devel_sgml, funcs_sgml],
    output: man_pages,
    command: [run_quiet, docbook2man, '@INPUT0@', '--output', '@OUTDIR@'],
    build_by_default: true,
    install_dir: get_option('mandir') / 'man3',
    install: true)

  # fonts.conf(5)
  custom_target('fonts-conf-5-man-page',
    input: [fontconfig_user_sgml],
    output: 'fonts-conf.5',
    command: [run_quiet, docbook2man, '@INPUT0@', '--output', '@OUTDIR@'],
    install_dir: get_option('mandir') / 'man5',
    build_by_default: true,
    install: true)

  # Generate man pages for tools
  foreach t : tools_man_pages
    # docbook2man doesn't seem to have a --quiet option unfortunately
    custom_target('@0@-man-page'.format(t),
      input: '../@0@/@0@.sgml'.format(t),
      output: '@0@.1'.format(t),
      command: [run_quiet, docbook2man, '@INPUT@', '--output', '@OUTDIR@'],
      install_dir: get_option('mandir') / 'man1',
      install: true)
  endforeach
endif

if docbook2pdf.found()
  doc_targets += ['PDF']

  custom_target('devel-pdf',
    input: [fontconfig_devel_sgml, funcs_sgml],
    output: 'fontconfig-devel.pdf',
    command: [run_quiet, docbook2pdf, '@INPUT0@', '--output', '@OUTDIR@'],
    build_by_default: true,
    install_dir: get_option('datadir') / 'doc' / 'fontconfig',
    install: true)

  custom_target('user-pdf',
    input: [fontconfig_user_sgml, funcs_sgml],
    output: 'fontconfig-user.pdf',
    command: [run_quiet, docbook2pdf, '@INPUT0@', '--output', '@OUTDIR@'],
    build_by_default: true,
    install_dir: get_option('datadir') / 'doc' / 'fontconfig',
    install: true)
endif

if docbook2txt.found()
  doc_targets += ['Text']

  custom_target('devel-txt',
    input: [fontconfig_devel_sgml, funcs_sgml],
    output: 'fontconfig-devel.txt',
    command: [run_quiet, docbook2txt, '@INPUT0@', '--output', '@OUTDIR@'],
    build_by_default: true,
    install_dir: get_option('datadir') / 'doc' / 'fontconfig',
    install: true)

  custom_target('user-txt',
    input: [fontconfig_user_sgml, funcs_sgml],
    output: 'fontconfig-user.txt',
    command: [run_quiet, docbook2txt, '@INPUT0@', '--output', '@OUTDIR@'],
    build_by_default: true,
    install_dir: get_option('datadir') / 'doc' / 'fontconfig',
    install: true)
endif

if docbook2html.found()
  doc_targets += ['HTML']

  custom_target('devel-html',
    input: [fontconfig_devel_sgml, funcs_sgml],
    output: 'fontconfig-devel.html',
    command: [run_quiet, docbook2html, '--nochunks', '@INPUT0@', '--output', '@OUTDIR@'],
    build_by_default: true,
    install_dir: get_option('datadir') / 'doc' / 'fontconfig',
    install: true)

  custom_target('user-html',
    input: [fontconfig_user_sgml, funcs_sgml],
    output: 'fontconfig-user.html',
    command: [run_quiet, docbook2html, '--nochunks', '@INPUT0@', '--output', '@OUTDIR@'],
    build_by_default: true,
    install_dir: get_option('datadir') / 'doc' / 'fontconfig',
    install: true)
endif
