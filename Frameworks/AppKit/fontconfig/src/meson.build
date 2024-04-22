fc_sources = [
  'fcatomic.c',
  'fccache.c',
  'fccfg.c',
  'fccharset.c',
  'fccompat.c',
  'fcdbg.c',
  'fcdefault.c',
  'fcdir.c',
  'fcformat.c',
  'fcfreetype.c',
  'fcfs.c',
  'fcptrlist.c',
  'fchash.c',
  'fcinit.c',
  'fclang.c',
  'fclist.c',
  'fcmatch.c',
  'fcmatrix.c',
  'fcname.c',
  'fcobjs.c',
  'fcpat.c',
  'fcrange.c',
  'fcserialize.c',
  'fcstat.c',
  'fcstr.c',
  'fcweight.c',
  'fcxml.c',
  'ftglue.c',
]

# FIXME: obviously fragile, cc.preprocess would be sweet
cpp = cc.cmd_array()
if cc.get_id() == 'gcc'
  cpp += ['-E', '-P']
elif cc.get_id() == 'msvc'
  cpp += ['/EP']
elif cc.get_id() == 'clang'
  cpp += ['-E', '-P']
else
  error('FIXME: implement cc.preprocess')
endif

cpp += ['-I', join_paths(meson.current_source_dir(), '..')]

fcobjshash_gperf = custom_target('fcobjshash.gperf',
  input: 'fcobjshash.gperf.h',
  output: 'fcobjshash.gperf',
  command: [python3, files('cutout.py')[0], '@INPUT@', '@OUTPUT@', '@BUILD_ROOT@', cpp],
  build_by_default: true,
)

fcobjshash_h = custom_target('fcobjshash.h',
  input: fcobjshash_gperf,
  output: 'fcobjshash.h',
  command: [gperf, '--pic', '-m', '100', '@INPUT@', '--output-file', '@OUTPUT@']
)

# Define FcPublic appropriately for exports on windows
fc_extra_c_args = []

if cc.get_argument_syntax() == 'msvc'
  fc_extra_c_args += '-DFcPublic=__declspec(dllexport)'
endif

libfontconfig = library('fontconfig',
  fc_sources, alias_headers, ft_alias_headers, fclang_h, fccase_h, fcobjshash_h,
  c_args: c_args + fc_extra_c_args,
  include_directories: incbase,
  dependencies: deps,
  install: true,
  soversion: soversion,
  version: libversion,
  darwin_versions: osxversion,
)

fontconfig_dep = declare_dependency(link_with: libfontconfig,
  include_directories: incbase,
  dependencies: deps,
)

pkgmod.generate(libfontconfig,
  description: 'Font configuration and customization library',
  filebase: 'fontconfig',
  name: 'Fontconfig',
  requires: ['freetype2 ' + freetype_req],
  version: fc_version,
  variables: [
    'sysconfdir=@0@'.format(join_paths(prefix, get_option('sysconfdir'))),
    'localstatedir=@0@'.format(join_paths(prefix, get_option('localstatedir'))),
    'confdir=@0@'.format(fc_baseconfigdir),
    'cachedir=@0@'.format(fc_cachedir),
  ])
