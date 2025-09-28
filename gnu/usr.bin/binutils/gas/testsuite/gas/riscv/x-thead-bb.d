#as: -march=rv64i_xtheadbb
#source: x-thead-bb.s
#objdump: -dr

.*:[ 	]+file format .*

Disassembly of section .text:

0+000 <target>:
[ 	]+[0-9a-f]+:[ 	]+1005950b[ 	]+th.srri[ 	]+a0,a1,0
[ 	]+[0-9a-f]+:[ 	]+1015950b[ 	]+th.srri[ 	]+a0,a1,1
[ 	]+[0-9a-f]+:[ 	]+13e5950b[ 	]+th.srri[ 	]+a0,a1,62
[ 	]+[0-9a-f]+:[ 	]+13f5950b[ 	]+th.srri[ 	]+a0,a1,63
[ 	]+[0-9a-f]+:[ 	]+1405950b[ 	]+th.srriw[ 	]+a0,a1,0
[ 	]+[0-9a-f]+:[ 	]+1415950b[ 	]+th.srriw[ 	]+a0,a1,1
[ 	]+[0-9a-f]+:[ 	]+15e5950b[ 	]+th.srriw[ 	]+a0,a1,30
[ 	]+[0-9a-f]+:[ 	]+15f5950b[ 	]+th.srriw[ 	]+a0,a1,31
[ 	]+[0-9a-f]+:[ 	]+0405a50b[ 	]+th.ext[ 	]+a0,a1,1,0
[ 	]+[0-9a-f]+:[ 	]+7c05a50b[ 	]+th.ext[ 	]+a0,a1,31,0
[ 	]+[0-9a-f]+:[ 	]+fdf5a50b[ 	]+th.ext[ 	]+a0,a1,63,31
[ 	]+[0-9a-f]+:[ 	]+ffe5a50b[ 	]+th.ext[ 	]+a0,a1,63,62
[ 	]+[0-9a-f]+:[ 	]+0405b50b[ 	]+th.extu[ 	]+a0,a1,1,0
[ 	]+[0-9a-f]+:[ 	]+7c05b50b[ 	]+th.extu[ 	]+a0,a1,31,0
[ 	]+[0-9a-f]+:[ 	]+fdf5b50b[ 	]+th.extu[ 	]+a0,a1,63,31
[ 	]+[0-9a-f]+:[ 	]+ffe5b50b[ 	]+th.extu[ 	]+a0,a1,63,62
[ 	]+[0-9a-f]+:[ 	]+8405950b[ 	]+th.ff0[ 	]+a0,a1
[ 	]+[0-9a-f]+:[ 	]+8605950b[ 	]+th.ff1[ 	]+a0,a1
[ 	]+[0-9a-f]+:[ 	]+8205950b[ 	]+th.rev[ 	]+a0,a1
[ 	]+[0-9a-f]+:[ 	]+9005950b[ 	]+th.revw[ 	]+a0,a1
[ 	]+[0-9a-f]+:[ 	]+8005950b[ 	]+th.tstnbz[ 	]+a0,a1
