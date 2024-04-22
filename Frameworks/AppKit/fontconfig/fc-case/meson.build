fccase_h = custom_target('fccase.h',
  output: 'fccase.h',
  input: ['CaseFolding.txt', 'fccase.tmpl.h'],
  command: [find_program('fc-case.py'), '@INPUT0@', '--template', '@INPUT1@', '--output', '@OUTPUT@'])
