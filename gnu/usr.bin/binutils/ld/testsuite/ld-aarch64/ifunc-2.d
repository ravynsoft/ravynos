#target: [check_shared_lib_support]
#ld: -shared --hash-style=sysv
#objdump: -dw

#...
0+(130|1a0|1c8) <foo>:
#...
[ \t0-9a-f]+:[ \t0-9a-f]+bl[ \t0-9a-f]+<\*ABS\*\+0x(130|1a0|1c8)@plt>
[ \t0-9a-f]+:[ \t0-9a-f]+adrp[ \t]+x0, 0 <.*>
[ \t0-9a-f]+:[ \t0-9a-f]+add[ \t]+x0, x0, #0x(120|190|1b8)
#pass
