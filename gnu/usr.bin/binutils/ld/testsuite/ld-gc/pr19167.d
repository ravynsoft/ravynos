#source: pr19167a.s
#source: pr19167b.s
#ld: --gc-sections -e _start
#objdump: -s -j _foo
#xfail: bfin-*-linux* frv-*-*linux* lm32-*-*linux*

#...
Contents of section _foo:
 [0-9a-f]+ [0-9a-f]+ [0-9a-f]+ [0-9a-f]+ [0-9a-f]+[ \t]+This is a test.*
#pass
