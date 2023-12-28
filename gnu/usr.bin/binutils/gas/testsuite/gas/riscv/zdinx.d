#as: -march=rv64i_zdinx
#objdump: -dr

.*:[ 	]+file format .*

Disassembly of section .text:

0+000 <target>:
[ 	]+[0-9a-f]+:[ 	]+02c5f553[ 	]+fadd.d[ 	]+a0,a1,a2
[ 	]+[0-9a-f]+:[ 	]+02c58553[ 	]+fadd.d[ 	]+a0,a1,a2,rne
[ 	]+[0-9a-f]+:[ 	]+0ac5f553[ 	]+fsub.d[ 	]+a0,a1,a2
[ 	]+[0-9a-f]+:[ 	]+0ac58553[ 	]+fsub.d[ 	]+a0,a1,a2,rne
[ 	]+[0-9a-f]+:[ 	]+12c5f553[ 	]+fmul.d[ 	]+a0,a1,a2
[ 	]+[0-9a-f]+:[ 	]+12c58553[ 	]+fmul.d[ 	]+a0,a1,a2,rne
[ 	]+[0-9a-f]+:[ 	]+1ac5f553[ 	]+fdiv.d[ 	]+a0,a1,a2
[ 	]+[0-9a-f]+:[ 	]+1ac58553[ 	]+fdiv.d[ 	]+a0,a1,a2,rne
[ 	]+[0-9a-f]+:[ 	]+5a05f553[ 	]+fsqrt.d[ 	]+a0,a1
[ 	]+[0-9a-f]+:[ 	]+5a058553[ 	]+fsqrt.d[ 	]+a0,a1,rne
[ 	]+[0-9a-f]+:[ 	]+2ac58553[ 	]+fmin.d[ 	]+a0,a1,a2
[ 	]+[0-9a-f]+:[ 	]+2ac59553[ 	]+fmax.d[ 	]+a0,a1,a2
[ 	]+[0-9a-f]+:[ 	]+6ac5f543[ 	]+fmadd.d[ 	]+a0,a1,a2,a3
[ 	]+[0-9a-f]+:[ 	]+6ac58543[ 	]+fmadd.d[ 	]+a0,a1,a2,a3,rne
[ 	]+[0-9a-f]+:[ 	]+6ac5f54f[ 	]+fnmadd.d[ 	]+a0,a1,a2,a3
[ 	]+[0-9a-f]+:[ 	]+6ac5854f[ 	]+fnmadd.d[ 	]+a0,a1,a2,a3,rne
[ 	]+[0-9a-f]+:[ 	]+6ac5f547[ 	]+fmsub.d[ 	]+a0,a1,a2,a3
[ 	]+[0-9a-f]+:[ 	]+6ac58547[ 	]+fmsub.d[ 	]+a0,a1,a2,a3,rne
[ 	]+[0-9a-f]+:[ 	]+6ac5f54b[ 	]+fnmsub.d[ 	]+a0,a1,a2,a3
[ 	]+[0-9a-f]+:[ 	]+6ac5854b[ 	]+fnmsub.d[ 	]+a0,a1,a2,a3,rne
[ 	]+[0-9a-f]+:[ 	]+c205f553[ 	]+fcvt.w.d[ 	]+a0,a1
[ 	]+[0-9a-f]+:[ 	]+c2058553[ 	]+fcvt.w.d[ 	]+a0,a1,rne
[ 	]+[0-9a-f]+:[ 	]+c215f553[ 	]+fcvt.wu.d[ 	]+a0,a1
[ 	]+[0-9a-f]+:[ 	]+c2158553[ 	]+fcvt.wu.d[ 	]+a0,a1,rne
[ 	]+[0-9a-f]+:[ 	]+c225f553[ 	]+fcvt.l.d[ 	]+a0,a1
[ 	]+[0-9a-f]+:[ 	]+c2258553[ 	]+fcvt.l.d[ 	]+a0,a1,rne
[ 	]+[0-9a-f]+:[ 	]+c235f553[ 	]+fcvt.lu.d[ 	]+a0,a1
[ 	]+[0-9a-f]+:[ 	]+c2358553[ 	]+fcvt.lu.d[ 	]+a0,a1,rne
[ 	]+[0-9a-f]+:[ 	]+d2058553[ 	]+fcvt.d.w[ 	]+a0,a1
[ 	]+[0-9a-f]+:[ 	]+d2158553[ 	]+fcvt.d.wu[ 	]+a0,a1
[ 	]+[0-9a-f]+:[ 	]+d225f553[ 	]+fcvt.d.l[ 	]+a0,a1
[ 	]+[0-9a-f]+:[ 	]+d2258553[ 	]+fcvt.d.l[ 	]+a0,a1,rne
[ 	]+[0-9a-f]+:[ 	]+d235f553[ 	]+fcvt.d.lu[ 	]+a0,a1
[ 	]+[0-9a-f]+:[ 	]+d2358553[ 	]+fcvt.d.lu[ 	]+a0,a1,rne
[ 	]+[0-9a-f]+:[ 	]+42058553[ 	]+fcvt.d.s[ 	]+a0,a1
[ 	]+[0-9a-f]+:[ 	]+4015f553[ 	]+fcvt.s.d[ 	]+a0,a1
[ 	]+[0-9a-f]+:[ 	]+40158553[ 	]+fcvt.s.d[ 	]+a0,a1,rne
[ 	]+[0-9a-f]+:[ 	]+22c58553[ 	]+fsgnj.d[ 	]+a0,a1,a2
[ 	]+[0-9a-f]+:[ 	]+22c59553[ 	]+fsgnjn.d[ 	]+a0,a1,a2
[ 	]+[0-9a-f]+:[ 	]+22c5a553[ 	]+fsgnjx.d[ 	]+a0,a1,a2
[ 	]+[0-9a-f]+:[ 	]+a2c5a553[ 	]+feq.d[ 	]+a0,a1,a2
[ 	]+[0-9a-f]+:[ 	]+a2c59553[ 	]+flt.d[ 	]+a0,a1,a2
[ 	]+[0-9a-f]+:[ 	]+a2c58553[ 	]+fle.d[ 	]+a0,a1,a2
[ 	]+[0-9a-f]+:[ 	]+a2b61553[ 	]+flt.d[ 	]+a0,a2,a1
[ 	]+[0-9a-f]+:[ 	]+a2b60553[ 	]+fle.d[ 	]+a0,a2,a1
[ 	]+[0-9a-f]+:[ 	]+22b58553[ 	]+fmv.d[ 	]+a0,a1
[ 	]+[0-9a-f]+:[ 	]+22b59553[ 	]+fneg.d[ 	]+a0,a1
[ 	]+[0-9a-f]+:[ 	]+22b5a553[ 	]+fabs.d[ 	]+a0,a1
[ 	]+[0-9a-f]+:[ 	]+e2059553[ 	]+fclass.d[ 	]+a0,a1
