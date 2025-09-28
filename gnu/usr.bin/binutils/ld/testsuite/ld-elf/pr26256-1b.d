#source: pr26256-1.s
#ld: -e _start
#nm: -n
# s12z uses memory regions
#xfail: s12z-*-*

#...
[0-9a-f]+ T _start
#pass
