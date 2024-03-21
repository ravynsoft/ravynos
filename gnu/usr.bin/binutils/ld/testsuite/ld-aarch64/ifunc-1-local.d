#target: [check_shared_lib_support]
#ld: -shared --hash-style=sysv
#objdump: -dw

#...
0+(110|180|1a0) <(__GI_)?foo>:
#...
[ \t0-9a-f]+:[ \t0-9a-f]+bl[ \t0-9a-f]+<\*ABS\*\+(0x110|0x180|0x1a0)@plt>
#pass
