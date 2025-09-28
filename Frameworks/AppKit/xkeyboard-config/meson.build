project('xkeyboard-config',
        version: '2.40',
        license: 'MIT/Expat',
        meson_version: '>= 0.54.0')

dir_data        = join_paths(get_option('prefix'), get_option('datadir'))
dir_xkb_base    = join_paths(dir_data, 'X11', 'xkb')
dir_xkb_rules   = join_paths(dir_xkb_base, 'rules')
dir_man7        = join_paths(get_option('prefix'), get_option('mandir'), 'man7')
dir_pkgconfig   = join_paths(dir_data, 'pkgconfig')

i18n = import('i18n')

pkgconfig = import('pkgconfig')
pkgconfig.generate(
    filebase: 'xkeyboard-config',
    name: 'XKeyboardConfig',
    description: 'X Keyboard configuration data',
    version: meson.project_version(),
    variables: [
        'datadir=@0@'.format(dir_data),
        'xkb_base=@0@'.format(dir_xkb_base),
    ],
    install_dir: dir_pkgconfig,
    dataonly: true,
)

# KcGST only need to be installed as-is
foreach dir: ['compat', 'geometry', 'keycodes', 'symbols', 'types']
    install_subdir(dir,
                   exclude_files: ['Makefile.am'],
                   install_dir: dir_xkb_base)
endforeach

# Rules are a bit more complicated
subdir('rules')

# man page
xsltproc = find_program('xsltproc', required: false)
if xsltproc.found()
  man_substs = configuration_data()
  man_substs.set('xkb_base', dir_xkb_base)
  # emulating what the macros do for vendorversion, hardcoding the man
  # suffixes
  man_substs.set('vendorversion', '"@0@ @1@" "X Version 11"'.format(meson.project_name(), meson.project_version()))
  xsl = configure_file(input: 'man/man.xsl',
                       output: 'man.xsl',
                       configuration: man_substs,
                       install: false)
  # evdev_ruleset is set by rules/meson.build
  manpage = custom_target('man page',
                          output: 'xkeyboard-config.7',
                          build_by_default: true,
                          command: [xsltproc, '-nonet', xsl, evdev_ruleset],
                          capture: true,
                          install:true,
                          install_dir: dir_man7)

  verify_group_names = find_program('tests/verify-group-names.sh')
  test('verify group names', verify_group_names)
endif

# pytest suite
pymod = import('python')
python = pymod.find_installation('python3',
                                 modules: ['pytest'],
                                 required: false)
pytest = find_program('pytest-3', 'pytest', required: false)
enable_pytest = python.found() and pytest.found()
if enable_pytest
    pytest_args = ['--verbose', '--log-level=DEBUG']
    # use pytest xdist if available, it really speeds up the tests cases
    optional_python_modules = ['xdist']
    if pymod.find_installation('python3', modules: optional_python_modules, required: false).found()
        pytest_args += ['-n', 'auto']
    endif

    # copy our data files over to the build directory so we can use the
    # builddir as XKB_CONFIG_ROOT
    foreach dir: ['compat', 'geometry', 'keycodes', 'symbols', 'types']
        run_command('tests/copydir.sh', dir, check: true)
    endforeach

    test('pytest', pytest,
         args: pytest_args,
         env: ['XKB_CONFIG_ROOT=@0@'.format(meson.build_root())],
         workdir: meson.source_root(),
         timeout: 60)
endif

subdir('po')
