project('cargo subproject', 'c')

foo_dep = dependency('foo-rs')
exe = executable('app', 'main.c',
  dependencies: foo_dep,
)
test('cargo-test', exe)
