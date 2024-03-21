#name: --gc-sections with other syms and start/stop syms
#ld: --gc-sections -e _start
#nm: -n
#target: [supports_gnu_unique]
#xfail: bfin-*-linux* frv-*-*linux* lm32-*-*linux*

#...
[0-9a-f]+ R +bar_xx
#...
