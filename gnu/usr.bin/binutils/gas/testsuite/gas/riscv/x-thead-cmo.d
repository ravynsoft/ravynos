#as: -march=rv64i_xtheadcmo
#source: x-thead-cmo.s
#objdump: -dr

.*:[ 	]+file format .*

Disassembly of section .text:

0+000 <target>:
[ 	]+[0-9a-f]+:[ 	]+0010000b[ 	]+th.dcache.call
[ 	]+[0-9a-f]+:[ 	]+0030000b[ 	]+th.dcache.ciall
[ 	]+[0-9a-f]+:[ 	]+0020000b[ 	]+th.dcache.iall
[ 	]+[0-9a-f]+:[ 	]+0295000b[ 	]+th.dcache.cpa[ 	]+a0
[ 	]+[0-9a-f]+:[ 	]+02b5800b[ 	]+th.dcache.cipa[ 	]+a1
[ 	]+[0-9a-f]+:[ 	]+02a6000b[ 	]+th.dcache.ipa[ 	]+a2
[ 	]+[0-9a-f]+:[ 	]+0256800b[ 	]+th.dcache.cva[ 	]+a3
[ 	]+[0-9a-f]+:[ 	]+0277000b[ 	]+th.dcache.civa[ 	]+a4
[ 	]+[0-9a-f]+:[ 	]+0267800b[ 	]+th.dcache.iva[ 	]+a5
[ 	]+[0-9a-f]+:[ 	]+0218000b[ 	]+th.dcache.csw[ 	]+a6
[ 	]+[0-9a-f]+:[ 	]+0238800b[ 	]+th.dcache.cisw[ 	]+a7
[ 	]+[0-9a-f]+:[ 	]+0222800b[ 	]+th.dcache.isw[ 	]+t0
[ 	]+[0-9a-f]+:[ 	]+0283000b[ 	]+th.dcache.cpal1[ 	]+t1
[ 	]+[0-9a-f]+:[ 	]+0243800b[ 	]+th.dcache.cval1[ 	]+t2
[ 	]+[0-9a-f]+:[ 	]+0100000b[ 	]+th.icache.iall
[ 	]+[0-9a-f]+:[ 	]+0110000b[ 	]+th.icache.ialls
[ 	]+[0-9a-f]+:[ 	]+038e000b[ 	]+th.icache.ipa[ 	]+t3
[ 	]+[0-9a-f]+:[ 	]+030e800b[ 	]+th.icache.iva[ 	]+t4
[ 	]+[0-9a-f]+:[ 	]+0150000b[ 	]+th.l2cache.call
[ 	]+[0-9a-f]+:[ 	]+0170000b[ 	]+th.l2cache.ciall
[ 	]+[0-9a-f]+:[ 	]+0160000b[ 	]+th.l2cache.iall
