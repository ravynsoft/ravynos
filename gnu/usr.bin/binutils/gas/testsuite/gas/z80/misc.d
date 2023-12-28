#objdump: -d
#name: miscellaneous instructions

.*: .*

Disassembly of section .text:

0+ <.text>:
[ 	]+[0-9a-f]+:[ 	]+08[ 	]+ex af,af'
[ 	]+[0-9a-f]+:[ 	]+d9[ 	]+exx
[ 	]+[0-9a-f]+:[ 	]+eb[ 	]+ex de,hl
[ 	]+[0-9a-f]+:[ 	]+e3[ 	]+ex \(sp\),hl
[ 	]+[0-9a-f]+:[ 	]+dd e3[ 	]+ex \(sp\),ix
[ 	]+[0-9a-f]+:[ 	]+fd e3[ 	]+ex \(sp\),iy
[ 	]+[0-9a-f]+:[ 	]+27[ 	]+daa
[ 	]+[0-9a-f]+:[ 	]+2f[ 	]+cpl
[ 	]+[0-9a-f]+:[ 	]+ed 44[ 	]+neg
[ 	]+[0-9a-f]+:[ 	]+3f[ 	]+ccf
[ 	]+[0-9a-f]+:[ 	]+37[ 	]+scf
[ 	]+[0-9a-f]+:[ 	]+00[ 	]+nop
[ 	]+[0-9a-f]+:[ 	]+76[ 	]+halt
[ 	]+[0-9a-f]+:[ 	]+f3[ 	]+di
[ 	]+[0-9a-f]+:[ 	]+fb[ 	]+ei
[ 	]+[0-9a-f]+:[ 	]+ed 46[ 	]+im 0
[ 	]+[0-9a-f]+:[ 	]+ed 56[ 	]+im 1
[ 	]+[0-9a-f]+:[ 	]+ed 5e[ 	]+im 2
