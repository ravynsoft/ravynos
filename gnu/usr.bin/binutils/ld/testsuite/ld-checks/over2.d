#name: section size overflow
#source: over2.s
#ld: -Ttext=0xfffffffc
#nm: -n

#...
fffffffc T _start
#pass
