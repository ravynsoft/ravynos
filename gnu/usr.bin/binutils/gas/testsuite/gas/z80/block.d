#objdump: -d
#name: block instructions

.*: .*

Disassembly of section .text:

0+ <.text>:
[ 	]+0:[ 	]+ed a0[ 	]+ldi
[ 	]+2:[ 	]+ed b0[ 	]+ldir
[ 	]+4:[ 	]+ed a8[ 	]+ldd
[ 	]+6:[ 	]+ed b8[ 	]+lddr
[ 	]+8:[ 	]+ed a1[ 	]+cpi
[ 	]+a:[ 	]+ed b1[ 	]+cpir
[ 	]+c:[ 	]+ed a9[ 	]+cpd
[ 	]+e:[ 	]+ed b9[ 	]+cpdr
[ 	]+10:[ 	]+ed a3[ 	]+outi
[ 	]+12:[ 	]+ed b3[ 	]+otir
[ 	]+14:[ 	]+ed ab[ 	]+outd
[ 	]+16:[ 	]+ed bb[ 	]+otdr
[ 	]+18:[ 	]+ed a2[ 	]+ini
[ 	]+1a:[ 	]+ed b2[ 	]+inir
[ 	]+1c:[ 	]+ed aa[ 	]+ind
[ 	]+1e:[ 	]+ed ba[ 	]+indr
#pass
