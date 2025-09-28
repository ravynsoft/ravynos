#as: -march=rv64i_zfinx
#objdump: -dr

.*:[ 	]+file format .*

Disassembly of section .text:

0+000 <target>:
[ 	]+[0-9a-f]+:[ 	]+00c5f553[ 	]+fadd.s[ 	]+a0,a1,a2
[ 	]+[0-9a-f]+:[ 	]+00c58553[ 	]+fadd.s[ 	]+a0,a1,a2,rne
[ 	]+[0-9a-f]+:[ 	]+08c5f553[ 	]+fsub.s[ 	]+a0,a1,a2
[ 	]+[0-9a-f]+:[ 	]+08c58553[ 	]+fsub.s[ 	]+a0,a1,a2,rne
[ 	]+[0-9a-f]+:[ 	]+10c5f553[ 	]+fmul.s[ 	]+a0,a1,a2
[ 	]+[0-9a-f]+:[ 	]+10c58553[ 	]+fmul.s[ 	]+a0,a1,a2,rne
[ 	]+[0-9a-f]+:[ 	]+18c5f553[ 	]+fdiv.s[ 	]+a0,a1,a2
[ 	]+[0-9a-f]+:[ 	]+18c58553[ 	]+fdiv.s[ 	]+a0,a1,a2,rne
[ 	]+[0-9a-f]+:[ 	]+5805f553[ 	]+fsqrt.s[ 	]+a0,a1
[ 	]+[0-9a-f]+:[ 	]+58058553[ 	]+fsqrt.s[ 	]+a0,a1,rne
[ 	]+[0-9a-f]+:[ 	]+28c58553[ 	]+fmin.s[ 	]+a0,a1,a2
[ 	]+[0-9a-f]+:[ 	]+28c59553[ 	]+fmax.s[ 	]+a0,a1,a2
[ 	]+[0-9a-f]+:[ 	]+68c5f543[ 	]+fmadd.s[ 	]+a0,a1,a2,a3
[ 	]+[0-9a-f]+:[ 	]+68c58543[ 	]+fmadd.s[ 	]+a0,a1,a2,a3,rne
[ 	]+[0-9a-f]+:[ 	]+68c5f54f[ 	]+fnmadd.s[ 	]+a0,a1,a2,a3
[ 	]+[0-9a-f]+:[ 	]+68c5854f[ 	]+fnmadd.s[ 	]+a0,a1,a2,a3,rne
[ 	]+[0-9a-f]+:[ 	]+68c5f547[ 	]+fmsub.s[ 	]+a0,a1,a2,a3
[ 	]+[0-9a-f]+:[ 	]+68c58547[ 	]+fmsub.s[ 	]+a0,a1,a2,a3,rne
[ 	]+[0-9a-f]+:[ 	]+68c5f54b[ 	]+fnmsub.s[ 	]+a0,a1,a2,a3
[ 	]+[0-9a-f]+:[ 	]+68c5854b[ 	]+fnmsub.s[ 	]+a0,a1,a2,a3,rne
[ 	]+[0-9a-f]+:[ 	]+c005f553[ 	]+fcvt.w.s[ 	]+a0,a1
[ 	]+[0-9a-f]+:[ 	]+c0058553[ 	]+fcvt.w.s[ 	]+a0,a1,rne
[ 	]+[0-9a-f]+:[ 	]+c015f553[ 	]+fcvt.wu.s[ 	]+a0,a1
[ 	]+[0-9a-f]+:[ 	]+c0158553[ 	]+fcvt.wu.s[ 	]+a0,a1,rne
[ 	]+[0-9a-f]+:[ 	]+c025f553[ 	]+fcvt.l.s[ 	]+a0,a1
[ 	]+[0-9a-f]+:[ 	]+c0258553[ 	]+fcvt.l.s[ 	]+a0,a1,rne
[ 	]+[0-9a-f]+:[ 	]+c035f553[ 	]+fcvt.lu.s[ 	]+a0,a1
[ 	]+[0-9a-f]+:[ 	]+c0358553[ 	]+fcvt.lu.s[ 	]+a0,a1,rne
[ 	]+[0-9a-f]+:[ 	]+d005f553[ 	]+fcvt.s.w[ 	]+a0,a1
[ 	]+[0-9a-f]+:[ 	]+d0058553[ 	]+fcvt.s.w[ 	]+a0,a1,rne
[ 	]+[0-9a-f]+:[ 	]+d015f553[ 	]+fcvt.s.wu[ 	]+a0,a1
[ 	]+[0-9a-f]+:[ 	]+d0158553[ 	]+fcvt.s.wu[ 	]+a0,a1,rne
[ 	]+[0-9a-f]+:[ 	]+d025f553[ 	]+fcvt.s.l[ 	]+a0,a1
[ 	]+[0-9a-f]+:[ 	]+d0258553[ 	]+fcvt.s.l[ 	]+a0,a1,rne
[ 	]+[0-9a-f]+:[ 	]+d035f553[ 	]+fcvt.s.lu[ 	]+a0,a1
[ 	]+[0-9a-f]+:[ 	]+d0358553[ 	]+fcvt.s.lu[ 	]+a0,a1,rne
[ 	]+[0-9a-f]+:[ 	]+20c58553[ 	]+fsgnj.s[ 	]+a0,a1,a2
[ 	]+[0-9a-f]+:[ 	]+20c59553[ 	]+fsgnjn.s[ 	]+a0,a1,a2
[ 	]+[0-9a-f]+:[ 	]+20c5a553[ 	]+fsgnjx.s[ 	]+a0,a1,a2
[ 	]+[0-9a-f]+:[ 	]+a0c5a553[ 	]+feq.s[ 	]+a0,a1,a2
[ 	]+[0-9a-f]+:[ 	]+a0c59553[ 	]+flt.s[ 	]+a0,a1,a2
[ 	]+[0-9a-f]+:[ 	]+a0c58553[ 	]+fle.s[ 	]+a0,a1,a2
[ 	]+[0-9a-f]+:[ 	]+a0b61553[ 	]+flt.s[ 	]+a0,a2,a1
[ 	]+[0-9a-f]+:[ 	]+a0b60553[ 	]+fle.s[ 	]+a0,a2,a1
[ 	]+[0-9a-f]+:[ 	]+20b58553[ 	]+fmv.s[ 	]+a0,a1
[ 	]+[0-9a-f]+:[ 	]+20b59553[ 	]+fneg.s[ 	]+a0,a1
[ 	]+[0-9a-f]+:[ 	]+20b5a553[ 	]+fabs.s[ 	]+a0,a1
[ 	]+[0-9a-f]+:[ 	]+e0059553[ 	]+fclass.s[ 	]+a0,a1
