#source: ifunc-3.s
#target: [check_shared_lib_support]
#ld: -shared --hash-style=sysv
#objdump: -dw

#...
0+(150|1d0|1e8) <__GI_foo>:
#...
[ \t0-9a-f]+:[ \t0-9a-f]+bl[ \t0-9a-f]+<\*ABS\*\+0x(150|1d0|1e8)@plt>
#pass
