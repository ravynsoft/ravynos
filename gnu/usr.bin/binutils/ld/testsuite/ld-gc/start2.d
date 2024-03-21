#name: --gc-sections with -z start-stop-gc
#ld: --gc-sections -e _start -z start-stop-gc
#nm: -n
#notarget: [uses_genelf]
#xfail: bfin-*-linux* frv-*-*linux*

#failif
#...
[0-9a-f]+ D +_?__start__foo
#...
