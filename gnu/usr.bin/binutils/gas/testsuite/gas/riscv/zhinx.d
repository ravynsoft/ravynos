#as: -march=rv64ima_zqinx_zhinx
#source: zhinx.s
#objdump: -dr

.*:[ 	]+file format .*


Disassembly of section .text:

0+000 <target>:
[ 	]+[0-9a-f]+:[ 	]+04c5f553[ 	]+fadd.h[ 	]+a0,a1,a2
[ 	]+[0-9a-f]+:[ 	]+04c58553[ 	]+fadd.h[ 	]+a0,a1,a2,rne
[ 	]+[0-9a-f]+:[ 	]+0cc5f553[ 	]+fsub.h[ 	]+a0,a1,a2
[ 	]+[0-9a-f]+:[ 	]+0cc58553[ 	]+fsub.h[ 	]+a0,a1,a2,rne
[ 	]+[0-9a-f]+:[ 	]+14c5f553[ 	]+fmul.h[ 	]+a0,a1,a2
[ 	]+[0-9a-f]+:[ 	]+14c58553[ 	]+fmul.h[ 	]+a0,a1,a2,rne
[ 	]+[0-9a-f]+:[ 	]+1cc5f553[ 	]+fdiv.h[ 	]+a0,a1,a2
[ 	]+[0-9a-f]+:[ 	]+1cc58553[ 	]+fdiv.h[ 	]+a0,a1,a2,rne
[ 	]+[0-9a-f]+:[ 	]+5c05f553[ 	]+fsqrt.h[ 	]+a0,a1
[ 	]+[0-9a-f]+:[ 	]+5c058553[ 	]+fsqrt.h[ 	]+a0,a1,rne
[ 	]+[0-9a-f]+:[ 	]+2cc58553[ 	]+fmin.h[ 	]+a0,a1,a2
[ 	]+[0-9a-f]+:[ 	]+2cc59553[ 	]+fmax.h[ 	]+a0,a1,a2
[ 	]+[0-9a-f]+:[ 	]+6cc5f543[ 	]+fmadd.h[ 	]+a0,a1,a2,a3
[ 	]+[0-9a-f]+:[ 	]+6cc58543[ 	]+fmadd.h[ 	]+a0,a1,a2,a3,rne
[ 	]+[0-9a-f]+:[ 	]+6cc5f54f[ 	]+fnmadd.h[ 	]+a0,a1,a2,a3
[ 	]+[0-9a-f]+:[ 	]+6cc5854f[ 	]+fnmadd.h[ 	]+a0,a1,a2,a3,rne
[ 	]+[0-9a-f]+:[ 	]+6cc5f547[ 	]+fmsub.h[ 	]+a0,a1,a2,a3
[ 	]+[0-9a-f]+:[ 	]+6cc58547[ 	]+fmsub.h[ 	]+a0,a1,a2,a3,rne
[ 	]+[0-9a-f]+:[ 	]+6cc5f54b[ 	]+fnmsub.h[ 	]+a0,a1,a2,a3
[ 	]+[0-9a-f]+:[ 	]+6cc5854b[ 	]+fnmsub.h[ 	]+a0,a1,a2,a3,rne
[ 	]+[0-9a-f]+:[ 	]+c405f553[ 	]+fcvt.w.h[ 	]+a0,a1
[ 	]+[0-9a-f]+:[ 	]+c4058553[ 	]+fcvt.w.h[ 	]+a0,a1,rne
[ 	]+[0-9a-f]+:[ 	]+c415f553[ 	]+fcvt.wu.h[ 	]+a0,a1
[ 	]+[0-9a-f]+:[ 	]+c4158553[ 	]+fcvt.wu.h[ 	]+a0,a1,rne
[ 	]+[0-9a-f]+:[ 	]+d405f553[ 	]+fcvt.h.w[ 	]+a0,a1
[ 	]+[0-9a-f]+:[ 	]+d4058553[ 	]+fcvt.h.w[ 	]+a0,a1,rne
[ 	]+[0-9a-f]+:[ 	]+d415f553[ 	]+fcvt.h.wu[ 	]+a0,a1
[ 	]+[0-9a-f]+:[ 	]+d4158553[ 	]+fcvt.h.wu[ 	]+a0,a1,rne
[ 	]+[0-9a-f]+:[ 	]+c425f553[ 	]+fcvt.l.h[ 	]+a0,a1
[ 	]+[0-9a-f]+:[ 	]+c4258553[ 	]+fcvt.l.h[ 	]+a0,a1,rne
[ 	]+[0-9a-f]+:[ 	]+c435f553[ 	]+fcvt.lu.h[ 	]+a0,a1
[ 	]+[0-9a-f]+:[ 	]+c4358553[ 	]+fcvt.lu.h[ 	]+a0,a1,rne
[ 	]+[0-9a-f]+:[ 	]+d425f553[ 	]+fcvt.h.l[ 	]+a0,a1
[ 	]+[0-9a-f]+:[ 	]+d4258553[ 	]+fcvt.h.l[ 	]+a0,a1,rne
[ 	]+[0-9a-f]+:[ 	]+d435f553[ 	]+fcvt.h.lu[ 	]+a0,a1
[ 	]+[0-9a-f]+:[ 	]+d4358553[ 	]+fcvt.h.lu[ 	]+a0,a1,rne
[ 	]+[0-9a-f]+:[ 	]+40260553[ 	]+fcvt.s.h[ 	]+a0,a2
[ 	]+[0-9a-f]+:[ 	]+42260553[ 	]+fcvt.d.h[ 	]+a0,a2
[ 	]+[0-9a-f]+:[ 	]+46260553[ 	]+fcvt.q.h[ 	]+a0,a2
[ 	]+[0-9a-f]+:[ 	]+44067553[ 	]+fcvt.h.s[ 	]+a0,a2
[ 	]+[0-9a-f]+:[ 	]+44060553[ 	]+fcvt.h.s[ 	]+a0,a2,rne
[ 	]+[0-9a-f]+:[ 	]+44167553[ 	]+fcvt.h.d[ 	]+a0,a2
[ 	]+[0-9a-f]+:[ 	]+44160553[ 	]+fcvt.h.d[ 	]+a0,a2,rne
[ 	]+[0-9a-f]+:[ 	]+44367553[ 	]+fcvt.h.q[ 	]+a0,a2
[ 	]+[0-9a-f]+:[ 	]+44360553[ 	]+fcvt.h.q[ 	]+a0,a2,rne
[ 	]+[0-9a-f]+:[ 	]+24c58553[ 	]+fsgnj.h[ 	]+a0,a1,a2
[ 	]+[0-9a-f]+:[ 	]+24c59553[ 	]+fsgnjn.h[ 	]+a0,a1,a2
[ 	]+[0-9a-f]+:[ 	]+24c5a553[ 	]+fsgnjx.h[ 	]+a0,a1,a2
[ 	]+[0-9a-f]+:[ 	]+a4c5a553[ 	]+feq.h[ 	]+a0,a1,a2
[ 	]+[0-9a-f]+:[ 	]+a4c59553[ 	]+flt.h[ 	]+a0,a1,a2
[ 	]+[0-9a-f]+:[ 	]+a4c58553[ 	]+fle.h[ 	]+a0,a1,a2
[ 	]+[0-9a-f]+:[ 	]+a4c59553[ 	]+flt.h[ 	]+a0,a1,a2
[ 	]+[0-9a-f]+:[ 	]+a4c58553[ 	]+fle.h[ 	]+a0,a1,a2
[ 	]+[0-9a-f]+:[ 	]+24b58553[ 	]+fmv.h[ 	]+a0,a1
[ 	]+[0-9a-f]+:[ 	]+24b59553[ 	]+fneg.h[ 	]+a0,a1
[ 	]+[0-9a-f]+:[ 	]+24b5a553[ 	]+fabs.h[ 	]+a0,a1
[ 	]+[0-9a-f]+:[ 	]+e4059553[ 	]+fclass.h[ 	]+a0,a1
