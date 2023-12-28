#name: --gc-sections with __start_
#ld: --gc-sections -e _start
#nm: -n
#xfail: bfin-*-linux* frv-*-*linux* lm32-*-*linux*

#...
[0-9a-f]+ D +_?__start__foo
#...
