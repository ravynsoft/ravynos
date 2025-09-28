#name: --gc-sections with groups and start/stop syms
#ld: --gc-sections -e _start
#nm: -n
#notarget: [is_generic]
#xfail: bfin-*-linux* frv-*-*linux* lm32-*-*linux*

#...
[0-9a-f]+ T +bar
#...
