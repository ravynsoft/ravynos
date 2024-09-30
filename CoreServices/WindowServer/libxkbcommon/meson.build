project(
    'libxkbcommon',
    'c',
    version: '1.6.0',
    default_options: [
        'c_std=c11',
        'warning_level=2',
        'b_lundef=true',
    ],
    meson_version : '>= 0.52.0',
)
pkgconfig = import('pkgconfig')
cc = meson.get_compiler('c')

dir_libexec = get_option('prefix')/get_option('libexecdir')/'xkbcommon'

# Compiler flags.
cflags = [
    '-fno-strict-aliasing',
    '-Wno-unused-parameter',
    '-Wno-missing-field-initializers',
    '-Wpointer-arith',
    '-Wmissing-declarations',
    '-Wformat=2',
    '-Wstrict-prototypes',
    '-Wmissing-prototypes',
    '-Wnested-externs',
    '-Wbad-function-cast',
    '-Wshadow',
    '-Wlogical-op',
    '-Wdate-time',
    '-Wwrite-strings',
    '-Wno-documentation-deprecated-sync',
]
add_project_arguments(cc.get_supported_arguments(cflags), language: 'c')


# The XKB config root.
XKBCONFIGROOT = get_option('xkb-config-root')
if XKBCONFIGROOT == ''
    xkeyboard_config_dep = dependency('xkeyboard-config', required: false)
    if xkeyboard_config_dep.found()
        XKBCONFIGROOT = xkeyboard_config_dep.get_variable(pkgconfig: 'xkb_base')
    else
        XKBCONFIGROOT = get_option('prefix')/get_option('datadir')/'X11'/'xkb'
  endif
endif

XKBCONFIGEXTRAPATH = get_option('xkb-config-extra-path')
if XKBCONFIGEXTRAPATH == ''
    XKBCONFIGEXTRAPATH = get_option('prefix')/get_option('sysconfdir')/'xkb'
endif

# The X locale directory for compose.
XLOCALEDIR = get_option('x-locale-root')
if XLOCALEDIR == ''
    XLOCALEDIR = get_option('prefix')/get_option('datadir')/'X11'/'locale'
endif


# config.h.
configh_data = configuration_data()
configh_data.set('EXIT_INVALID_USAGE', '2')
configh_data.set_quoted('LIBXKBCOMMON_VERSION', meson.project_version())
configh_data.set_quoted('LIBXKBCOMMON_TOOL_PATH', dir_libexec)
# Like AC_USE_SYSTEM_EXTENSIONS, what #define to use to get extensions
# beyond the base POSIX function set.
if host_machine.system() == 'sunos'
  system_extensions = '__EXTENSIONS__'
else
  system_extensions = '_GNU_SOURCE'
endif
configh_data.set(system_extensions, 1)
system_ext_define = '#define ' + system_extensions
configh_data.set_quoted('DFLT_XKB_CONFIG_ROOT', XKBCONFIGROOT)
configh_data.set_quoted('DFLT_XKB_CONFIG_EXTRA_PATH', XKBCONFIGEXTRAPATH)
configh_data.set_quoted('XLOCALEDIR', XLOCALEDIR)
configh_data.set_quoted('DEFAULT_XKB_RULES', get_option('default-rules'))
configh_data.set_quoted('DEFAULT_XKB_MODEL', get_option('default-model'))
configh_data.set_quoted('DEFAULT_XKB_LAYOUT', get_option('default-layout'))
if get_option('default-variant') != ''
    configh_data.set_quoted('DEFAULT_XKB_VARIANT', get_option('default-variant'))
else
    configh_data.set('DEFAULT_XKB_VARIANT', 'NULL')
endif
if get_option('default-options') != ''
    configh_data.set_quoted('DEFAULT_XKB_OPTIONS', get_option('default-options'))
else
    configh_data.set('DEFAULT_XKB_OPTIONS', 'NULL')
endif
if cc.has_header('unistd.h')
    configh_data.set('HAVE_UNISTD_H', 1)
endif
if cc.links('int main(){if(__builtin_expect(1<0,0)){}}', name: '__builtin_expect')
    configh_data.set('HAVE___BUILTIN_EXPECT', 1)
endif
if cc.has_header_symbol('unistd.h', 'eaccess', prefix: system_ext_define)
    configh_data.set('HAVE_EACCESS', 1)
endif
if cc.has_header_symbol('unistd.h', 'euidaccess', prefix: system_ext_define)
    configh_data.set('HAVE_EUIDACCESS', 1)
endif
if cc.has_header_symbol('sys/mman.h', 'mmap')
    configh_data.set('HAVE_MMAP', 1)
endif
if cc.has_header_symbol('stdlib.h', 'mkostemp', prefix: system_ext_define)
    configh_data.set('HAVE_MKOSTEMP', 1)
endif
if cc.has_header_symbol('fcntl.h', 'posix_fallocate', prefix: system_ext_define)
    configh_data.set('HAVE_POSIX_FALLOCATE', 1)
endif
if cc.has_header_symbol('string.h', 'strndup', prefix: system_ext_define)
    configh_data.set('HAVE_STRNDUP', 1)
endif
if cc.has_header_symbol('stdio.h', 'asprintf', prefix: system_ext_define)
    configh_data.set('HAVE_ASPRINTF', 1)
elif cc.has_header_symbol('stdio.h', 'vasprintf', prefix: system_ext_define)
    configh_data.set('HAVE_VASPRINTF', 1)
endif
if cc.has_header_symbol('stdlib.h', 'secure_getenv', prefix: system_ext_define)
    configh_data.set('HAVE_SECURE_GETENV', 1)
elif cc.has_header_symbol('stdlib.h', '__secure_getenv', prefix: system_ext_define)
    configh_data.set('HAVE___SECURE_GETENV', 1)
else
    message('C library does not support secure_getenv, using getenv instead')
endif
if not cc.has_header_symbol('limits.h', 'PATH_MAX', prefix: system_ext_define)
    if host_machine.system() == 'windows'
        # see https://docs.microsoft.com/en-us/windows/win32/fileio/naming-a-file#maximum-path-length-limitation
        configh_data.set('PATH_MAX', 260)
    else
        configh_data.set('PATH_MAX', 4096)
    endif
endif

# Silence some security & deprecation warnings on MSVC
# for some unix/C functions we use.
# https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-3-c4996?view=vs-2019
configh_data.set('_CRT_SECURE_NO_WARNINGS', 1)
configh_data.set('_CRT_NONSTDC_NO_WARNINGS', 1)
configh_data.set('_CRT_NONSTDC_NO_DEPRECATE', 1)
# Reduce unnecessary includes on MSVC.
configh_data.set('WIN32_LEAN_AND_MEAN', 1)

# Supports -Wl,--version-script?
have_version_script = cc.links(
    'int main(){}',
    args: '-Wl,--version-script=' + meson.current_source_dir()/'xkbcommon.map',
    name: '-Wl,--version-script',
)

map_to_def = find_program('scripts/map-to-def')

# libxkbcommon.
# Note: we use some yacc extensions, which work with either GNU bison
# (preferred) or byacc (with backtracking enabled).
bison = find_program('bison', 'win_bison', required: false, version: '>= 2.3a')
if bison.found()
    yacc = bison
    yacc_gen = generator(
        bison,
        output: ['@BASENAME@.c', '@BASENAME@.h'],
        arguments: ['--defines=@OUTPUT1@', '-o', '@OUTPUT0@', '-p', '_xkbcommon_', '@INPUT@'],
    )
else
    byacc = find_program('byacc', required: false)
    if byacc.found()
        yacc = byacc
        yacc_gen = generator(
            byacc,
            output: ['@BASENAME@.c', '@BASENAME@.h'],
            arguments: ['-H', '@OUTPUT1@', '-o', '@OUTPUT0@', '-p', '_xkbcommon_', '@INPUT@'],
        )
    else
        error('Could not find a compatible YACC program (bison or byacc)')
    endif
endif
libxkbcommon_sources = [
    'src/compose/parser.c',
    'src/compose/parser.h',
    'src/compose/paths.c',
    'src/compose/paths.h',
    'src/compose/state.c',
    'src/compose/table.c',
    'src/compose/table.h',
    'src/xkbcomp/action.c',
    'src/xkbcomp/action.h',
    'src/xkbcomp/ast.h',
    'src/xkbcomp/ast-build.c',
    'src/xkbcomp/ast-build.h',
    'src/xkbcomp/compat.c',
    'src/xkbcomp/expr.c',
    'src/xkbcomp/expr.h',
    'src/xkbcomp/include.c',
    'src/xkbcomp/include.h',
    'src/xkbcomp/keycodes.c',
    'src/xkbcomp/keymap.c',
    'src/xkbcomp/keymap-dump.c',
    'src/xkbcomp/keywords.c',
    yacc_gen.process('src/xkbcomp/parser.y'),
    'src/xkbcomp/parser-priv.h',
    'src/xkbcomp/rules.c',
    'src/xkbcomp/rules.h',
    'src/xkbcomp/scanner.c',
    'src/xkbcomp/symbols.c',
    'src/xkbcomp/types.c',
    'src/xkbcomp/vmod.c',
    'src/xkbcomp/vmod.h',
    'src/xkbcomp/xkbcomp.c',
    'src/xkbcomp/xkbcomp-priv.h',
    'src/atom.c',
    'src/atom.h',
    'src/context.c',
    'src/context.h',
    'src/context-priv.c',
    'src/darray.h',
    'src/keysym.c',
    'src/keysym.h',
    'src/keysym-utf.c',
    'src/ks_tables.h',
    'src/keymap.c',
    'src/keymap.h',
    'src/keymap-priv.c',
    'src/messages-codes.h',
    'src/scanner-utils.h',
    'src/state.c',
    'src/text.c',
    'src/text.h',
    'src/utf8.c',
    'src/utf8.h',
    'src/utils.c',
    'src/utils.h',
]
libxkbcommon_link_args = []
libxkbcommon_link_deps = []
if have_version_script
    libxkbcommon_link_args += '-Wl,--version-script=' + meson.current_source_dir()/'xkbcommon.map'
    libxkbcommon_link_deps += 'xkbcommon.map'
elif cc.get_argument_syntax() == 'msvc'
    libxkbcommon_def = custom_target('xkbcommon.def',
        command: [map_to_def, '@INPUT@', '@OUTPUT@'],
        input: 'xkbcommon.map',
        output: 'kxbcommon.def',
    )
    libxkbcommon_link_deps += libxkbcommon_def
    libxkbcommon_link_args += '/DEF:' + libxkbcommon_def.full_path()
endif
libxkbcommon = library(
    'xkbcommon',
    'include/xkbcommon/xkbcommon.h',
    libxkbcommon_sources,
    link_args: libxkbcommon_link_args,
    link_depends: libxkbcommon_link_deps,
    gnu_symbol_visibility: 'hidden',
    version: '0.0.0',
    install: true,
    include_directories: include_directories('src', 'include'),
)
install_headers(
    'include/xkbcommon/xkbcommon.h',
    'include/xkbcommon/xkbcommon-compat.h',
    'include/xkbcommon/xkbcommon-compose.h',
    'include/xkbcommon/xkbcommon-keysyms.h',
    'include/xkbcommon/xkbcommon-names.h',
    subdir: 'xkbcommon',
)

dep_libxkbcommon = declare_dependency(
    link_with: libxkbcommon,
    include_directories: include_directories('include'),
)
if meson.version().version_compare('>= 0.54.0')
    meson.override_dependency('xkbcommon', dep_libxkbcommon)
endif
pkgconfig.generate(
    libxkbcommon,
    name: 'xkbcommon',
    filebase: 'xkbcommon',
    version: meson.project_version(),
    description: 'XKB API common to servers and clients',
)


# libxkbcommon-x11.
if get_option('enable-x11')
    xcb_dep = dependency('xcb', version: '>=1.10', required: false)
    xcb_xkb_dep = dependency('xcb-xkb', version: '>=1.10', required: false)
    if not xcb_dep.found() or not xcb_xkb_dep.found()
        error('''X11 support requires xcb-xkb >= 1.10 which was not found.
You can disable X11 support with -Denable-x11=false.''')
    endif

    libxkbcommon_x11_sources = [
        'src/x11/keymap.c',
        'src/x11/state.c',
        'src/x11/util.c',
        'src/x11/x11-priv.h',
        'src/context.h',
        'src/context-priv.c',
        'src/keymap.h',
        'src/keymap-priv.c',
        'src/atom.h',
        'src/atom.c',
    ]
    libxkbcommon_x11_link_args = []
    libxkbcommon_x11_link_deps = []
    if have_version_script
        libxkbcommon_x11_link_args += '-Wl,--version-script=' + meson.current_source_dir()/'xkbcommon-x11.map'
        libxkbcommon_x11_link_deps += 'xkbcommon-x11.map'
    elif cc.get_argument_syntax() == 'msvc'
        libxkbcommon_x11_def = custom_target('xkbcommon-x11.def',
            command: [map_to_def, '@INPUT@', '@OUTPUT@'],
            input: 'xkbcommon-x11.map',
            output: 'xkbcommon-x11.def',
        )
        libxkbcommon_x11_link_deps += libxkbcommon_x11_def
        libxkbcommon_x11_link_args += '/DEF:' + libxkbcommon_x11_def.full_path()
    endif
    libxkbcommon_x11 = library(
        'xkbcommon-x11',
        'include/xkbcommon/xkbcommon-x11.h',
        libxkbcommon_x11_sources,
        link_args: libxkbcommon_x11_link_args,
        link_depends: libxkbcommon_x11_link_deps,
        gnu_symbol_visibility: 'hidden',
        version: '0.0.0',
        install: true,
        include_directories: include_directories('src', 'include'),
        link_with: libxkbcommon,
        dependencies: [
            xcb_dep,
            xcb_xkb_dep,
        ],
    )
    install_headers(
        'include/xkbcommon/xkbcommon-x11.h',
        subdir: 'xkbcommon',
    )
    dep_libxkbcommon_x11 = declare_dependency(
        link_with: libxkbcommon_x11,
        include_directories: include_directories('include'),
    )
    if meson.version().version_compare('>= 0.54.0')
        meson.override_dependency('xkbcommon-x11', dep_libxkbcommon_x11)
    endif
    pkgconfig.generate(
        libxkbcommon_x11,
        name: 'xkbcommon-x11',
        filebase: 'xkbcommon-x11',
        version: meson.project_version(),
        description: 'XKB API common to servers and clients - X11 support',
        requires: ['xkbcommon'],
        requires_private: ['xcb>=1.10', 'xcb-xkb>=1.10'],
    )
endif

# libxkbregistry
if get_option('enable-xkbregistry')
    dep_libxml = dependency('libxml-2.0')
    deps_libxkbregistry = [dep_libxml]
    libxkbregistry_sources = [
        'src/registry.c',
        'src/utils.h',
        'src/utils.c',
        'src/util-list.h',
        'src/util-list.c',
    ]
    libxkbregistry_link_args = []
    libxkbregistry_link_deps = []
    if have_version_script
        libxkbregistry_link_args += '-Wl,--version-script=' + meson.current_source_dir()/'xkbregistry.map'
        libxkbregistry_link_deps += 'xkbregistry.map'
    elif cc.get_argument_syntax() == 'msvc'
        libxkbregistry_def = custom_target('xkbregistry.def',
            command: [map_to_def, '@INPUT@', '@OUTPUT@'],
            input: 'xkbregistry.map',
            output: 'xkbregistry.def',
        )
        libxkbregistry_link_deps += libxkbregistry_def
        libxkbregistry_link_args += '/DEF:' + libxkbregistry_def.full_path()
    endif
    libxkbregistry = library(
        'xkbregistry',
        'include/xkbcommon/xkbregistry.h',
        libxkbregistry_sources,
        link_args: libxkbregistry_link_args,
        link_depends: libxkbregistry_link_deps,
        gnu_symbol_visibility: 'hidden',
        dependencies: deps_libxkbregistry,
        version: '0.0.0',
        install: true,
        include_directories: include_directories('src', 'include'),
    )
    install_headers(
        'include/xkbcommon/xkbregistry.h',
        subdir: 'xkbcommon',
    )
    pkgconfig.generate(
        libxkbregistry,
        name: 'xkbregistry',
        filebase: 'xkbregistry',
        version: meson.project_version(),
        description: 'XKB API to query available rules, models, layouts, variants and options',
    )

    dep_libxkbregistry = declare_dependency(
        link_with: libxkbregistry,
        include_directories: include_directories('include'),
    )
    if meson.version().version_compare('>= 0.54.0')
        meson.override_dependency('xkbregistry', dep_libxkbregistry)
    endif
endif

man_pages = []

# Tools
build_tools = get_option('enable-tools') and cc.has_header_symbol('getopt.h', 'getopt_long', prefix: '#define _GNU_SOURCE')
if build_tools
    # Common resources
    libxkbcommon_tools_internal_sources = [
        'tools/tools-common.h',
        'tools/tools-common.c',
    ]
    libxkbcommon_tools_internal = static_library(
        'tools-internal',
        libxkbcommon_tools_internal_sources,
        dependencies: dep_libxkbcommon,
    )
    tools_dep = declare_dependency(
        include_directories: [include_directories('tools', 'include')],
        link_with: libxkbcommon_tools_internal,
        dependencies: dep_libxkbcommon,
    )

    # Tool: xkbcli
    executable('xkbcli', 'tools/xkbcli.c',
               dependencies: tools_dep, install: true)
    install_man('tools/xkbcli.1')

    if get_option('enable-bash-completion')
        bash_completion_path = get_option('bash-completion-path')
        if bash_completion_path == ''
            bash_completion = dependency('bash-completion', required: false)
            if bash_completion.found()
                bash_completion_path = bash_completion.get_variable(pkgconfig: 'completionsdir')
            else
                bash_completion_path = get_option('datadir') / 'bash-completion/completions'
            endif
        endif
        install_data('tools/xkbcli-bash-completion.sh',
                     rename: 'xkbcli',
                     install_dir: bash_completion_path)
    endif

    # Tool: compile-keymap
    xkbcli_compile_keymap = executable('xkbcli-compile-keymap',
                                       'tools/compile-keymap.c',
                                       dependencies: tools_dep,
                                       install: true,
                                       install_dir: dir_libexec)
    install_man('tools/xkbcli-compile-keymap.1')
    # The same tool again, but with access to some private APIs.
    executable('compile-keymap',
               'tools/compile-keymap.c',
               libxkbcommon_sources,
               dependencies: [tools_dep],
               c_args: ['-DENABLE_PRIVATE_APIS'],
               include_directories: [include_directories('src', 'include')],
               install: false)

    # Tool: compose
    executable('compose',
               'tools/compose.c',
               dependencies: tools_dep,
               include_directories: [include_directories('src', 'include')],
               install: false)
    configh_data.set10('HAVE_XKBCLI_COMPILE_KEYMAP', true)

    # Tool: how-to-type
    executable('xkbcli-how-to-type',
               'tools/how-to-type.c',
               dependencies: tools_dep,
               install: true,
               install_dir: dir_libexec)
    install_man('tools/xkbcli-how-to-type.1')
    configh_data.set10('HAVE_XKBCLI_HOW_TO_TYPE', true)

    # Tool: interactive-evdev
    if cc.has_header('linux/input.h')
        executable('xkbcli-interactive-evdev',
                   'tools/interactive-evdev.c',
                   dependencies: tools_dep,
                   install: true,
                   install_dir: dir_libexec)
        configh_data.set10('HAVE_XKBCLI_INTERACTIVE_EVDEV', true)
        install_man('tools/xkbcli-interactive-evdev.1')
        # The same tool again, but with access to some private APIs.
        executable('interactive-evdev',
                'tools/interactive-evdev.c',
                libxkbcommon_sources,
                libxkbcommon_tools_internal_sources,
                dependencies: [tools_dep],
                c_args: ['-DENABLE_PRIVATE_APIS'],
                include_directories: [include_directories('src', 'include')],
                install: false)
    endif

    # Tool: interactive-x11
    if get_option('enable-x11')
        x11_tools_dep = declare_dependency(
            link_with: libxkbcommon_x11,
            dependencies: [
                tools_dep,
                xcb_dep,
                xcb_xkb_dep,
            ],
        )
        executable('xkbcli-interactive-x11',
                   'tools/interactive-x11.c',
                   dependencies: x11_tools_dep,
                   install: true,
                   install_dir: dir_libexec)
        install_man('tools/xkbcli-interactive-x11.1')
        configh_data.set10('HAVE_XKBCLI_INTERACTIVE_X11', true)
    endif

    # Tool: interactive-wayland
    if get_option('enable-wayland')
        wayland_client_dep = dependency('wayland-client', version: '>=1.2.0', required: false)
        wayland_protocols_dep = dependency('wayland-protocols', version: '>=1.12', required: false)
        wayland_scanner_dep = dependency('wayland-scanner', required: false, native: true)
        if not wayland_client_dep.found() or not wayland_protocols_dep.found() or not wayland_scanner_dep.found()
            error('''The Wayland xkbcli programs require wayland-client >= 1.2.0, wayland-protocols >= 1.7 which were not found.
You can disable the Wayland xkbcli programs with -Denable-wayland=false.''')
        endif

        wayland_scanner = find_program(wayland_scanner_dep.get_variable(pkgconfig: 'wayland_scanner'))
        wayland_scanner_code_gen = generator(
            wayland_scanner,
            output: '@BASENAME@-protocol.c',
            arguments: ['private-code', '@INPUT@', '@OUTPUT@'],
        )
        wayland_scanner_client_header_gen = generator(
            wayland_scanner,
            output: '@BASENAME@-client-protocol.h',
            arguments: ['client-header', '@INPUT@', '@OUTPUT@'],
        )
        wayland_protocols_datadir = wayland_protocols_dep.get_variable(pkgconfig: 'pkgdatadir')
        xdg_shell_xml = wayland_protocols_datadir/'stable/xdg-shell/xdg-shell.xml'
        xdg_shell_sources = [
            wayland_scanner_code_gen.process(xdg_shell_xml),
            wayland_scanner_client_header_gen.process(xdg_shell_xml),
        ]
        executable('xkbcli-interactive-wayland',
                   'tools/interactive-wayland.c',
                   xdg_shell_sources,
                   dependencies: [tools_dep, wayland_client_dep],
                   install: true,
                   install_dir: dir_libexec)
        install_man('tools/xkbcli-interactive-wayland.1')
        configh_data.set10('HAVE_XKBCLI_INTERACTIVE_WAYLAND', true)
    endif

    # Tool: list
    if get_option('enable-xkbregistry')
        configh_data.set10('HAVE_XKBCLI_LIST', true)
        executable('xkbcli-list',
                   'tools/registry-list.c',
                   dependencies: dep_libxkbregistry,
                   install: true,
                   install_dir: dir_libexec)
        install_man('tools/xkbcli-list.1')
    endif

    # Tool: check-messages
    executable('xkb-check-messages',
               'tools/check-messages.c',
               'tools/messages.c',
               'tools/messages.h',
               'src/messages-codes.h',
               dependencies: [tools_dep],
               include_directories: [include_directories('src', 'include', 'tools')],
               install: false)
endif


# xkeyboard-config "verifier"
xkct_config = configuration_data()
xkct_config.set('MESON_BUILD_ROOT', meson.current_build_dir())
xkct_config.set('XKB_CONFIG_ROOT', XKBCONFIGROOT)
configure_file(input: 'test/xkeyboard-config-test.py.in',
               output: 'xkeyboard-config-test',
               configuration: xkct_config)

# Tests
test_env = environment()
test_env.set('XKB_LOG_LEVEL', 'debug')
test_env.set('XKB_LOG_VERBOSITY', '10')
test_env.set('top_srcdir', meson.current_source_dir())
test_env.set('top_builddir', meson.current_build_dir())
test_env.set('HAVE_XKBCLI_INTERACTIVE_EVDEV', configh_data.get('HAVE_XKBCLI_INTERACTIVE_EVDEV', 0).to_string())
test_env.set('HAVE_XKBCLI_INTERACTIVE_WAYLAND', configh_data.get('HAVE_XKBCLI_INTERACTIVE_WAYLAND', 0).to_string())
test_env.set('HAVE_XKBCLI_INTERACTIVE_X11', configh_data.get('HAVE_XKBCLI_INTERACTIVE_X11', 0).to_string())
test_env.set('HAVE_XKBCLI_LIST', configh_data.get('HAVE_XKBCLI_LIST', 0).to_string())

test_configh_data = configuration_data()
test_configh_data.set_quoted('TEST_XKB_CONFIG_ROOT', meson.current_source_dir()/'test'/'data')
configure_file(output: 'test-config.h', configuration: test_configh_data)

# Some tests need to use unexported symbols, so we link them against
# an internal copy of libxkbcommon with all symbols exposed.
libxkbcommon_test_internal = static_library(
    'xkbcommon-test-internal',
    'test/common.c',
    'test/test.h',
    'test/evdev-scancodes.h',
    'bench/bench.c',
    'bench/bench.h',
    libxkbcommon_sources,
    include_directories: include_directories('src', 'include'),
)
test_dep = declare_dependency(
    include_directories: include_directories('src', 'include'),
    link_with: libxkbcommon_test_internal,
)
if get_option('enable-x11')
    libxkbcommon_x11_test_internal = static_library(
        'xkbcommon-x11-internal',
        libxkbcommon_x11_sources,
        'test/xvfb-wrapper.c',
        'test/xvfb-wrapper.h',
        include_directories: include_directories('src', 'include'),
        link_with: libxkbcommon_test_internal,
        dependencies: [
            xcb_dep,
            xcb_xkb_dep,
        ],
    )
    x11_test_dep = declare_dependency(
        link_with: libxkbcommon_x11_test_internal,
        dependencies: [
            test_dep,
            xcb_dep,
            xcb_xkb_dep,
        ],
    )
endif
test(
    'keysym',
    executable('test-keysym', 'test/keysym.c', dependencies: test_dep),
    env: test_env,
)
test(
    'keymap',
    executable('test-keymap', 'test/keymap.c', dependencies: test_dep),
    env: test_env,
)
test(
    'filecomp',
    executable('test-filecomp', 'test/filecomp.c', dependencies: test_dep),
    env: test_env,
)
test(
    'context',
    executable('test-context', 'test/context.c', dependencies: test_dep),
    env: test_env,
)
test(
    'rules-file',
    executable('test-rules-file', 'test/rules-file.c', dependencies: test_dep),
    env: test_env,
)
test(
    'rules-file-includes',
    executable('test-rules-file-includes', 'test/rules-file-includes.c', dependencies: test_dep),
    env: test_env,
)
test(
    'stringcomp',
    executable('test-stringcomp', 'test/stringcomp.c', dependencies: test_dep),
    env: test_env,
)
test(
    'buffercomp',
    executable('test-buffercomp', 'test/buffercomp.c', dependencies: test_dep),
    env: test_env,
)
test(
    'log',
    executable('test-log', 'test/log.c', dependencies: test_dep),
    env: test_env,
)
test(
    'atom',
    executable('test-atom', 'test/atom.c', dependencies: test_dep),
    env: test_env,
)
test(
    'utf8',
    executable('test-utf8', 'test/utf8.c', dependencies: test_dep),
    env: test_env,
)
test(
    'state',
    executable('test-state', 'test/state.c', dependencies: test_dep),
    env: test_env,
)
test(
    'keyseq',
    executable('test-keyseq', 'test/keyseq.c', dependencies: test_dep),
    env: test_env,
)
test(
    'rulescomp',
    executable('test-rulescomp', 'test/rulescomp.c', dependencies: test_dep),
    env: test_env,
)
test(
    'compose',
    executable('test-compose', 'test/compose.c', dependencies: test_dep),
    env: test_env,
)
test(
    'utils',
    executable('test-utils', 'test/utils.c', dependencies: test_dep),
    env: test_env,
)
test(
    'symbols-leak-test',
    find_program('test/symbols-leak-test.py'),
    env: test_env,
    suite: ['python-tests'],
)
test(
    'modifiers',
    executable('test-modifiers', 'test/modifiers.c', dependencies: test_dep),
    env: test_env,
)
test(
    'messages',
    executable(
        'test-messages',
        'test/messages.c',
        'tools/messages.c',
        'tools/messages.h',
        include_directories: include_directories('src', 'include', 'tools'),
        dependencies: test_dep),
    env: test_env,
)
if get_option('enable-x11')
    test(
        'x11',
        executable('test-x11', 'test/x11.c', dependencies: x11_test_dep),
        env: test_env,
    )
    test(
        'x11comp',
        executable('test-x11comp', 'test/x11comp.c', dependencies: x11_test_dep),
        env: test_env,
    )
endif
if get_option('enable-xkbregistry')
    test(
        'registry',
        executable('test-registry', 'test/registry.c',
                   include_directories: include_directories('src'),
                   dependencies: [dep_libxkbregistry, test_dep]),
        env: test_env,
    )
endif
if build_tools
    test('tool-option-parsing',
         find_program('test/tool-option-parsing.py'),
         env: test_env,
         suite: ['python-tests'])

    # A set of keysyms to test for. Add one or two symbols to this array
    # whenever the xorgproto gets updated to make sure we resolve them.
    keysyms_to_test = [
        'XF86Macro23',
    ]

    env = environment()
    env.set('XKB_CONFIG_ROOT', meson.current_source_dir()/'test'/'data')
    foreach keysym: keysyms_to_test
        test('keysym-test-@0@'.format(keysym),
             find_program('test/test-keysym.py'),
             env: env,
             args: [keysym, '--tool', xkbcli_compile_keymap],
             suite: ['python-tests'])
    endforeach
endif

valgrind = find_program('valgrind', required: false)
if valgrind.found()
    add_test_setup('valgrind',
        exe_wrapper: [valgrind,
                       '--leak-check=full',
                       '--track-origins=yes',
                       '--gen-suppressions=all',
                       '--error-exitcode=99'],
        timeout_multiplier : 10)
else
    message('valgrind not found, disabling valgrind test setup')
endif


# Fuzzing target programs.
executable('fuzz-keymap', 'fuzz/keymap/target.c', dependencies: test_dep)
executable('fuzz-compose', 'fuzz/compose/target.c', dependencies: test_dep)


# Benchmarks.
bench_env = environment()
bench_env.set('top_srcdir', meson.current_source_dir())
benchmark(
    'key-proc',
    executable('bench-key-proc', 'bench/key-proc.c', dependencies: test_dep),
    env: bench_env,
)
benchmark(
    'rules',
    executable('bench-rules', 'bench/rules.c', dependencies: test_dep),
    env: bench_env,
)
benchmark(
    'rulescomp',
    executable('bench-rulescomp', 'bench/rulescomp.c', dependencies: test_dep),
    env: bench_env,
)
benchmark(
    'compose',
    executable('bench-compose', 'bench/compose.c', dependencies: test_dep),
    env: bench_env,
)
benchmark(
    'compose-traversal',
    executable('bench-compose-traversal', 'bench/compose-traversal.c', dependencies: test_dep),
    env: bench_env,
)
benchmark(
    'atom',
    executable('bench-atom', 'bench/atom.c', dependencies: test_dep),
    env: bench_env,
)
if get_option('enable-x11')
  benchmark(
      'x11',
      executable('bench-x11', 'bench/x11.c', dependencies: x11_test_dep),
      env: bench_env,
  )
endif


# Documentation.
if get_option('enable-docs')
    doxygen = find_program('doxygen', required: false)
    if not doxygen.found()
        error('''Documentation requires doxygen which was not found.
You can disable the documentation with -Denable-docs=false.''')
    endif
    doxygen_wrapper = find_program('scripts/doxygen-wrapper')

    doxygen_input = [
        'README.md',
        'doc/diagrams/xkb-configuration.dot',
        'doc/doxygen-extra.css',
        'doc/introduction-to-xkb.md',
        'doc/quick-guide.md',
        'doc/compatibility.md',
        'doc/user-configuration.md',
        'doc/rules-format.md',
        'doc/keymap-format-text-v1.md',
        'doc/message-registry.md',
        'include/xkbcommon/xkbcommon.h',
        'include/xkbcommon/xkbcommon-compose.h',
        'include/xkbcommon/xkbcommon-keysyms.h',
        'include/xkbcommon/xkbcommon-names.h',
        'include/xkbcommon/xkbcommon-x11.h',
        'include/xkbcommon/xkbregistry.h',
    ]
    doxygen_data = configuration_data()
    doxygen_data.set('PACKAGE_NAME', meson.project_name())
    doxygen_data.set('PACKAGE_VERSION', meson.project_version())
    doxygen_data.set('INPUT', ' '.join(doxygen_input))
    doxygen_data.set('OUTPUT_DIRECTORY', meson.current_build_dir())
    doxyfile = configure_file(
        input: 'doc/Doxyfile.in',
        output: 'Doxyfile',
        configuration: doxygen_data,
    )
    # TODO: Meson should provide this.
    docdir = get_option('datadir')/'doc'/meson.project_name()
    doc_gen = custom_target(
        'doc',
        input: [doxyfile] + doxygen_input,
        output: 'html',
        command: [
            doxygen_wrapper,
            doxygen,
            meson.current_build_dir()/'Doxyfile',
            meson.current_source_dir(),
        ],
        install: true,
        install_dir: docdir,
        build_by_default: true,
    )
    if get_option('enable-cool-uris')
        ensure_stable_urls = find_program('scripts'/'ensure-stable-doc-urls.py')
        custom_target(
            'doc-cool-uris',
            input: [doc_gen, 'doc'/'cool-uris.yaml'],
            output: 'html-xtra',
            command: [
                ensure_stable_urls,
                'generate-redirections',
                meson.current_source_dir()/'doc'/'cool-uris.yaml',
                meson.current_build_dir()/'html'
            ],
            install: false,
            build_by_default: true,
        )
    endif
endif

configure_file(output: 'config.h', configuration: configh_data)


# Stable variables for projects using xkbcommon as a subproject.
# These variables should not be renamed.
libxkbcommon_dep = dep_libxkbcommon
if get_option('enable-x11')
  libxkbcommon_x11_dep = dep_libxkbcommon_x11
endif
if get_option('enable-xkbregistry')
  libxkbregistry_dep = dep_libxkbregistry
endif

if meson.version().version_compare('>=0.62.0')
    summary({
      'backend': meson.backend(),
      'buildtype': get_option('buildtype'),
      'c_args': get_option('c_args'),
      'c_link_args': get_option('c_link_args'),
      'yacc': yacc.full_path() + ' ' + yacc.version(),
    }, section: 'Compiler')
    summary({
      'prefix': get_option('prefix'),
      'bindir': get_option('bindir'),
      'libdir': get_option('libdir'),
      'datadir': get_option('datadir'),
      'xkb-config-root': XKBCONFIGROOT,
      'xkb-config-extra-path': XKBCONFIGEXTRAPATH,
      'xlocaledir': XLOCALEDIR,
    }, section: 'Directories')
    summary({
      'docs': get_option('enable-docs'),
      'tools': get_option('enable-tools'),
      'wayland': get_option('enable-wayland'),
      'x11': get_option('enable-x11'),
    }, section: 'Features')
    summary({
      'layout': get_option('default-layout'),
      'model': get_option('default-model'),
      'options': get_option('default-options'),
      'rules': get_option('default-rules'),
      'variant': get_option('default-variant'),
    }, section: 'Defaults')
endif
