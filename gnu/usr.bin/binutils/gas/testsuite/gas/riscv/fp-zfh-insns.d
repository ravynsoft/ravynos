#as: -march=rv64ifdq_zfh
#source: fp-zfh-insns.s
#objdump: -dr

.*:[ 	]+file format .*


Disassembly of section .text:

0+000 <.text>:
[ 	]+[0-9a-f]+:[ 	]+00059507[ 	]+flh[ 	]+fa0,0\(a1\)
[ 	]+[0-9a-f]+:[ 	]+00a59027[ 	]+fsh[ 	]+fa0,0\(a1\)
[ 	]+[0-9a-f]+:[ 	]+24b58553[ 	]+fmv.h[ 	]+fa0,fa1
[ 	]+[0-9a-f]+:[ 	]+24b59553[ 	]+fneg.h[ 	]+fa0,fa1
[ 	]+[0-9a-f]+:[ 	]+24b5a553[ 	]+fabs.h[ 	]+fa0,fa1
[ 	]+[0-9a-f]+:[ 	]+24c58553[ 	]+fsgnj.h[ 	]+fa0,fa1,fa2
[ 	]+[0-9a-f]+:[ 	]+24c59553[ 	]+fsgnjn.h[ 	]+fa0,fa1,fa2
[ 	]+[0-9a-f]+:[ 	]+24c5a553[ 	]+fsgnjx.h[ 	]+fa0,fa1,fa2
[ 	]+[0-9a-f]+:[ 	]+04c5f553[ 	]+fadd.h[ 	]+fa0,fa1,fa2
[ 	]+[0-9a-f]+:[ 	]+04c58553[ 	]+fadd.h[ 	]+fa0,fa1,fa2,rne
[ 	]+[0-9a-f]+:[ 	]+0cc5f553[ 	]+fsub.h[ 	]+fa0,fa1,fa2
[ 	]+[0-9a-f]+:[ 	]+0cc58553[ 	]+fsub.h[ 	]+fa0,fa1,fa2,rne
[ 	]+[0-9a-f]+:[ 	]+14c5f553[ 	]+fmul.h[ 	]+fa0,fa1,fa2
[ 	]+[0-9a-f]+:[ 	]+14c58553[ 	]+fmul.h[ 	]+fa0,fa1,fa2,rne
[ 	]+[0-9a-f]+:[ 	]+1cc5f553[ 	]+fdiv.h[ 	]+fa0,fa1,fa2
[ 	]+[0-9a-f]+:[ 	]+1cc58553[ 	]+fdiv.h[ 	]+fa0,fa1,fa2,rne
[ 	]+[0-9a-f]+:[ 	]+5c05f553[ 	]+fsqrt.h[ 	]+fa0,fa1
[ 	]+[0-9a-f]+:[ 	]+5c058553[ 	]+fsqrt.h[ 	]+fa0,fa1,rne
[ 	]+[0-9a-f]+:[ 	]+2cc58553[ 	]+fmin.h[ 	]+fa0,fa1,fa2
[ 	]+[0-9a-f]+:[ 	]+2cc59553[ 	]+fmax.h[ 	]+fa0,fa1,fa2
[ 	]+[0-9a-f]+:[ 	]+6cc5f543[ 	]+fmadd.h[ 	]+fa0,fa1,fa2,fa3
[ 	]+[0-9a-f]+:[ 	]+6cc58543[ 	]+fmadd.h[ 	]+fa0,fa1,fa2,fa3,rne
[ 	]+[0-9a-f]+:[ 	]+6cc5f54f[ 	]+fnmadd.h[ 	]+fa0,fa1,fa2,fa3
[ 	]+[0-9a-f]+:[ 	]+6cc5854f[ 	]+fnmadd.h[ 	]+fa0,fa1,fa2,fa3,rne
[ 	]+[0-9a-f]+:[ 	]+6cc5f547[ 	]+fmsub.h[ 	]+fa0,fa1,fa2,fa3
[ 	]+[0-9a-f]+:[ 	]+6cc58547[ 	]+fmsub.h[ 	]+fa0,fa1,fa2,fa3,rne
[ 	]+[0-9a-f]+:[ 	]+6cc5f54b[ 	]+fnmsub.h[ 	]+fa0,fa1,fa2,fa3
[ 	]+[0-9a-f]+:[ 	]+6cc5854b[ 	]+fnmsub.h[ 	]+fa0,fa1,fa2,fa3,rne
[ 	]+[0-9a-f]+:[ 	]+c405f553[ 	]+fcvt.w.h[ 	]+a0,fa1
[ 	]+[0-9a-f]+:[ 	]+c4058553[ 	]+fcvt.w.h[ 	]+a0,fa1,rne
[ 	]+[0-9a-f]+:[ 	]+c415f553[ 	]+fcvt.wu.h[ 	]+a0,fa1
[ 	]+[0-9a-f]+:[ 	]+c4158553[ 	]+fcvt.wu.h[ 	]+a0,fa1,rne
[ 	]+[0-9a-f]+:[ 	]+d405f553[ 	]+fcvt.h.w[ 	]+fa0,a1
[ 	]+[0-9a-f]+:[ 	]+d4058553[ 	]+fcvt.h.w[ 	]+fa0,a1,rne
[ 	]+[0-9a-f]+:[ 	]+d415f553[ 	]+fcvt.h.wu[ 	]+fa0,a1
[ 	]+[0-9a-f]+:[ 	]+d4158553[ 	]+fcvt.h.wu[ 	]+fa0,a1,rne
[ 	]+[0-9a-f]+:[ 	]+c425f553[ 	]+fcvt.l.h[ 	]+a0,fa1
[ 	]+[0-9a-f]+:[ 	]+c4258553[ 	]+fcvt.l.h[ 	]+a0,fa1,rne
[ 	]+[0-9a-f]+:[ 	]+c435f553[ 	]+fcvt.lu.h[ 	]+a0,fa1
[ 	]+[0-9a-f]+:[ 	]+c4358553[ 	]+fcvt.lu.h[ 	]+a0,fa1,rne
[ 	]+[0-9a-f]+:[ 	]+d425f553[ 	]+fcvt.h.l[ 	]+fa0,a1
[ 	]+[0-9a-f]+:[ 	]+d4258553[ 	]+fcvt.h.l[ 	]+fa0,a1,rne
[ 	]+[0-9a-f]+:[ 	]+d435f553[ 	]+fcvt.h.lu[ 	]+fa0,a1
[ 	]+[0-9a-f]+:[ 	]+d4358553[ 	]+fcvt.h.lu[ 	]+fa0,a1,rne
[ 	]+[0-9a-f]+:[ 	]+e4058553[ 	]+fmv.x.h[ 	]+a0,fa1
[ 	]+[0-9a-f]+:[ 	]+f4058553[ 	]+fmv.h.x[ 	]+fa0,a1
[ 	]+[0-9a-f]+:[ 	]+40258553[ 	]+fcvt.s.h[ 	]+fa0,fa1
[ 	]+[0-9a-f]+:[ 	]+42258553[ 	]+fcvt.d.h[ 	]+fa0,fa1
[ 	]+[0-9a-f]+:[ 	]+46258553[ 	]+fcvt.q.h[ 	]+fa0,fa1
[ 	]+[0-9a-f]+:[ 	]+4405f553[ 	]+fcvt.h.s[ 	]+fa0,fa1
[ 	]+[0-9a-f]+:[ 	]+44058553[ 	]+fcvt.h.s[ 	]+fa0,fa1,rne
[ 	]+[0-9a-f]+:[ 	]+4415f553[ 	]+fcvt.h.d[ 	]+fa0,fa1
[ 	]+[0-9a-f]+:[ 	]+44158553[ 	]+fcvt.h.d[ 	]+fa0,fa1,rne
[ 	]+[0-9a-f]+:[ 	]+4435f553[ 	]+fcvt.h.q[ 	]+fa0,fa1
[ 	]+[0-9a-f]+:[ 	]+44358553[ 	]+fcvt.h.q[ 	]+fa0,fa1,rne
[ 	]+[0-9a-f]+:[ 	]+e4059553[ 	]+fclass.h[ 	]+a0,fa1
[ 	]+[0-9a-f]+:[ 	]+a4c5a553[ 	]+feq.h[ 	]+a0,fa1,fa2
[ 	]+[0-9a-f]+:[ 	]+a4c59553[ 	]+flt.h[ 	]+a0,fa1,fa2
[ 	]+[0-9a-f]+:[ 	]+a4c58553[ 	]+fle.h[ 	]+a0,fa1,fa2
[ 	]+[0-9a-f]+:[ 	]+a4c59553[ 	]+flt.h[ 	]+a0,fa1,fa2
[ 	]+[0-9a-f]+:[ 	]+a4c58553[ 	]+fle.h[ 	]+a0,fa1,fa2
