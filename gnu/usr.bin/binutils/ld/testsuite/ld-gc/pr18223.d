#name: --gc-sections with .text._init
#ld: --gc-sections -shared
#nm: -n
#xfail: tic6x-*-*

#...
[0-9a-f]+ t +_init
#...
