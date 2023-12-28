#target: [check_shared_lib_support]
#ld: -shared --hash-style=sysv
#objdump: -dw

#...
0+(110|180|1a0) <__GI_foo>:
#...
[ \t0-9a-f]+:[ \t0-9a-f]+bl[ \t0-9a-f]+<\*ABS\*\+0x(110|180|1a0)@plt>
[ \t0-9a-f]+:[ \t0-9a-f]+adrp[ \t]+x0, 0 <.*>
[ \t0-9a-f]+:[ \t0-9a-f]+add[ \t]+x0, x0, #0x(100|170|190)
#pass
