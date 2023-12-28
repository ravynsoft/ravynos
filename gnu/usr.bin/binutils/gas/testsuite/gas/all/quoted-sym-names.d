#nm: --extern-only --numeric-sort
#name: quoted symbol names
# No quoted strings handling (TC_STRING_ESCAPES set to 0):
#notarget: powerpc*-*-aix* powerpc*-*-beos* powerpc-*-macos* rs6000-*-*
# Explicitly no escapes in quoted strings:
#notarget: z80-*-*

#...
0+00 D test-a
0+01 D back\\slash
0+02 D back"slash
0+03 D backslash\\
0+04 D backslash"
