#source: advsimd-mov-bad.s
#readelf: -s --wide
# This test is only valid on ELF based ports.
#notarget: *-*-*coff *-*-pe *-*-wince *-*-*aout* *-*-netbsd

Symbol table '.symtab' contains 6 entries:
 +Num:.*
 +[0-9]+:.*
 +[0-9]+:.*
 +[0-9]+:.*
 +[0-9]+:.*
 +[0-9]+:.*
#failif
 +[0-9]+: +[0-9a-f]+ +0 +NOTYPE +GLOBAL +DEFAULT +UND v0.D
