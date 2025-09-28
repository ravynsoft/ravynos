tests = [
  ['test-bz89617.c', {'c_args': ['-DSRCDIR="@0@"'.format(meson.current_source_dir())]}],
  ['test-bz131804.c'],
  ['test-bz96676.c'],
  ['test-name-parse.c'],
  ['test-bz106618.c'],
  ['test-bz1744377.c'],
  ['test-issue180.c'],
  ['test-family-matching.c'],
]

if host_machine.system() != 'windows'
  tests += [
    # FIXME: ['test-migration.c'],
    ['test-bz106632.c', {'c_args': ['-DFONTFILE="@0@"'.format(join_paths(meson.current_source_dir(), '4x6.pcf'))]}],
    ['test-issue107.c'], # FIXME: fails on mingw
    # FIXME: this needs NotoSans-hinted.zip font downloaded and unpacked into test build directory! see run-test.sh
    ['test-crbug1004254.c', {'dependencies': dependency('threads')}], # for pthread
  ]

  if get_option('default_library') == 'static'
    tests += [
      ['test-issue110.c'],
      ['test-d1f48f11.c'],
    ]
  endif
endif

foreach test_data : tests
  fname = test_data[0]
  opts = test_data.length() > 1 ? test_data[1] : {}
  extra_c_args = opts.get('c_args', [])
  extra_deps = opts.get('dependencies', [])

  test_name = fname.split('.')[0].underscorify()
  exe = executable(test_name, fname,
    c_args: c_args + extra_c_args,
    include_directories: incbase,
    link_with: [libfontconfig],
    dependencies: extra_deps,
  )

  test(test_name, exe, timeout: 600)
endforeach

# FIXME: run-test.sh stuff
# FIXME: jsonc test-conf
