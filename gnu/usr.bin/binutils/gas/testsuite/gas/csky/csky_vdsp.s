
.text
vdsp_instructions:
   vstrq.8 vr2, (r2, r3 << 0)
   vstrq.16 vr2, (r2, r3 << 0)
   vstrq.32 vr2, (r2, r3 << 0)
   vldrq.8 vr2, (r2, r3 << 0)
   vldrq.16 vr2, (r2, r3 << 0)
   vldrq.32 vr2, (r2, r3 << 0)
   vstrd.8 vr2, (r2, r3 << 0)
   vstrd.16 vr2, (r2, r3 << 0)
   vstrd.32 vr2, (r2, r3 << 0)
   vldrd.8 vr2, (r2, r3 << 0)
   vldrd.16 vr2, (r2, r3 << 0)
   vldrd.32 vr2, (r2, r3 << 0)
   vldq.8  vr2, (r2, 16)
   vldq.16  vr2, (r2, 16)
   vldq.32  vr2, (r2, 16)
   vstq.8  vr2, (r2, 16)
   vstq.16  vr2, (r2, 16)
   vstq.32  vr2, (r2, 16)
   vldd.8  vr2, (r2, 16)
   vldd.16  vr2, (r2, 16)
   vldd.32  vr2, (r2, 16)
   vstd.8  vr2, (r2, 16)
   vstd.16  vr2, (r2, 16)
   vstd.32  vr2, (r2, 16)
  vmulsh  r30, r1
  vmulsha r0,  r31
  vmulsw  r12, r12
  vmulswa r28, r21
  vmulsws r1,  r4
  vmfvr.u8   r1,  vr2[9]
  vmfvr.u16  r3,  fr4[0]
  vmfvr.u32  r31, vr8[5]
  vmfvr.s8   r13, fr4[1]
  vmfvr.s16  r23, vr15[13]
  vmtvr.u8   vr5[0],  r16
  vmtvr.u16  fr4[7],  r10
  vmtvr.u32  vr15 [ 15 ] ,  r10
  vdup.8     vr1,  vr10[10]
  vdup.16    vr15,  fr10[1]
  vdup.32    fr7,  fr10[5]
    vmov         vr2, vr3
    vcadd.eu8    vr2, vr3
    vcadd.eu16    vr2, vr3
    vcadd.es8     vr2, vr3
    vcadd.es16    vr2, vr3
    vmov.eu8      vr2, vr3
    vmov.eu16     vr2, vr3
    vmov.es8      vr2, vr3
    vmov.es16     vr2, vr3
    vmov.u16.l    vr2, vr3
    vmov.u32.l    vr2, vr3
    vmov.s16.l    vr2, vr3
    vmov.s32.l    vr2, vr3
    vmov.u16.sl    vr2, vr3
    vmov.u32.sl    vr2, vr3
    vmov.s16.sl    vr2, vr3
    vmov.s32.sl    vr2, vr3
    vmov.u16.h    vr2, vr3
    vmov.u32.h    vr2, vr3
    vmov.s16.h    vr2, vr3
    vmov.s32.h    vr2, vr3
    vmov.u16.rh    vr2, vr3
    vmov.u32.rh    vr2, vr3
    vmov.s16.rh    vr2, vr3
    vmov.s32.rh    vr2, vr3
    vstou.u16.sl    vr2, vr3
    vstou.u32.sl    vr2, vr3
    vstou.s16.sl    vr2, vr3
    vstou.s32.sl    vr2, vr3
    vrev.8        vr2, vr3
    vrev.16       vr2, vr3
    vrev.32       vr2, vr3
    vcnt1.8       vr2, vr3
    vclz.8        vr2, vr3
    vclz.16       vr2, vr3
    vclz.32       vr2, vr3
    vcls.u8       vr2, vr3
    vcls.u16      vr2, vr3
    vcls.u32      vr2, vr3
    vcls.s8       vr2, vr3
    vcls.s16      vr2, vr3
    vcls.s32      vr2, vr3
    vabs.s8       vr2, vr3
    vabs.s16      vr2, vr3
    vabs.s32      vr2, vr3
    vabs.u8.s     vr2, vr3
    vabs.u16.s    vr2, vr3
    vabs.u32.s    vr2, vr3
    vabs.s8.s     vr2, vr3
    vabs.s16.s    vr2, vr3
    vabs.s32.s    vr2, vr3
    vneg.u8       vr2, vr3
    vneg.u16      vr2, vr3
    vneg.u32      vr2, vr3
    vneg.s8       vr2, vr3
    vneg.s16      vr2, vr3
    vneg.s32      vr2, vr3
    vneg.u8.s     vr2, vr3
    vneg.u16.s    vr2, vr3
    vneg.u32.s    vr2, vr3
    vneg.s8.s     vr2, vr3
    vneg.s16.s    vr2, vr3
    vneg.s32.s    vr2, vr3
    vcmphsz.u8    vr2, vr3
    vcmphsz.u16    vr2, vr3
    vcmphsz.u32    vr2, vr3
    vcmphsz.s8    vr2, vr3
    vcmphsz.s16    vr2, vr3
    vcmphsz.s32    vr2, vr3
    vcmpltz.u8    vr2, vr3
    vcmpltz.u16    vr2, vr3
    vcmpltz.u32    vr2, vr3
    vcmpltz.s8    vr2, vr3
    vcmpltz.s16    vr2, vr3
    vcmpltz.s32    vr2, vr3
    vcmpnez.u8    vr2, vr3
    vcmpnez.u16    vr2, vr3
    vcmpnez.u32    vr2, vr3
    vcmpnez.s8     vr2, vr3
    vcmpnez.s8     vr2, vr3
    vcmpnez.s16    vr2, vr3
    vcmpnez.s32    vr2, vr3
    vtrch.8         vr2, vr3, vr4
    vtrch.16         vr2, vr3, vr4
    vtrch.32          vr2, vr3, vr4
    vtrcl.8           vr2, vr3, vr4
    vtrcl.16          vr2, vr3, vr4
    vtrcl.32          vr2, vr3, vr4
    vadd.u8           vr2, vr3, vr4
    vadd.u16          vr2, vr3, vr4
    vadd.u32          vr2, vr3, vr4
    vadd.s8           vr2, vr3, vr4
    vadd.s16          vr2, vr3, vr4
    vadd.s32          vr2, vr3, vr4
    vadd.eu8          vr2, vr3, vr4
    vadd.eu16          vr2, vr3, vr4
    vadd.es8           vr2, vr3, vr4
    vadd.es16          vr2, vr3, vr4
    vcadd.u8           vr2, vr3, vr4
    vcadd.u16          vr2, vr3, vr4
    vcadd.u32          vr2, vr3, vr4
    vcadd.s8           vr2, vr3, vr4
    vcadd.s16          vr2, vr3, vr4
    vcadd.s32          vr2, vr3, vr4
    vadd.xu16.sl      vr2, vr3, vr4
    vadd.xu32.sl      vr2, vr3, vr4
    vadd.xs16.sl      vr2, vr3, vr4
    vadd.xs32.sl      vr2, vr3, vr4
    vadd.xu16         vr2, vr3, vr4
    vadd.xu32         vr2, vr3, vr4
    vadd.xs16         vr2, vr3, vr4
    vadd.xs32         vr2, vr3, vr4
    vaddh.u8          vr2, vr3, vr4
    vaddh.u16        vr2, vr3, vr4
    vaddh.u32        vr2, vr3, vr4
    vaddh.s8         vr2, vr3, vr4
    vaddh.s16        vr2, vr3, vr4
    vaddh.s32        vr2, vr3, vr4
    vaddh.u8.r       vr2, vr3, vr4
    vaddh.u16.r      vr2, vr3, vr4
    vaddh.u32.r      vr2, vr3, vr4
    vaddh.s8.r       vr2, vr3, vr4
    vaddh.s16.r      vr2, vr3, vr4
    vaddh.s32.r      vr2, vr3, vr4
    vadd.u8.s        vr2, vr3, vr4
    vadd.u16.s       vr2, vr3, vr4
    vadd.u32.s       vr2, vr3, vr4
    vadd.s8.s        vr2, vr3, vr4
    vadd.s16.s       vr2, vr3, vr4
    vadd.s32.s       vr2, vr3, vr4
    vsub.u8          vr2, vr3, vr4
    vsub.u16         vr2, vr3, vr4
    vsub.u32         vr2, vr3, vr4
    vsub.s8          vr2, vr3, vr4
    vsub.s16         vr2, vr3, vr4
    vsub.s32         vr2, vr3, vr4
    vsub.eu8         vr2, vr3, vr4
    vsub.eu16        vr2, vr3, vr4
    vsub.es8         vr2, vr3, vr4
    vsub.es16        vr2, vr3, vr4
    vsabs.u8         vr2, vr3, vr4
    vsabs.u16        vr2, vr3, vr4
    vsabs.u32        vr2, vr3, vr4
    vsabs.s8         vr2, vr3, vr4
    vsabs.s16        vr2, vr3, vr4
    vsabs.s32        vr2, vr3, vr4
    vsabs.eu8        vr2, vr3, vr4
    vsabs.eu16       vr2, vr3, vr4
    vsabs.es8        vr2, vr3, vr4
    vsabs.es16       vr2, vr3, vr4
    vsabsa.u8        vr2, vr3, vr4
    vsabsa.u16       vr2, vr3, vr4
    vsabsa.u32       vr2, vr3, vr4
    vsabsa.s8        vr2, vr3, vr4
    vsabsa.s16       vr2, vr3, vr4
    vsabsa.s32       vr2, vr3, vr4
    vsabsa.eu8       vr2, vr3, vr4
    vsabsa.eu16      vr2, vr3, vr4
    vsabsa.es8       vr2, vr3, vr4
    vsabsa.es16      vr2, vr3, vr4
    vsub.xu16        vr2, vr3, vr4
    vsub.xu32        vr2, vr3, vr4
    vsub.xs16        vr2, vr3, vr4
    vsub.xs32        vr2, vr3, vr4
    vsubh.u8         vr2, vr3, vr4
    vsubh.u16        vr2, vr3, vr4
    vsubh.u32        vr2, vr3, vr4
    vsubh.s8         vr2, vr3, vr4
    vsubh.s16        vr2, vr3, vr4
    vsubh.s32        vr2, vr3, vr4
    vsubh.u8.r       vr2, vr3, vr4
    vsubh.u16.r      vr2, vr3, vr4
    vsubh.u32.r      vr2, vr3, vr4
    vsubh.s8.r       vr2, vr3, vr4
    vsubh.s16.r      vr2, vr3, vr4
    vsubh.s32.r      vr2, vr3, vr4
    vsub.u8.s        vr2, vr3, vr4
    vsub.u16.s       vr2, vr3, vr4
    vsub.u32.s       vr2, vr3, vr4
    vsub.s8.s        vr2, vr3, vr4
    vsub.s16.s       vr2, vr3, vr4
    vsub.s32.s       vr2, vr3, vr4
    vmul.u8          vr2, vr3, vr4
    vmul.u16         vr2, vr3, vr4
    vmul.u32         vr2, vr3, vr4
    vmul.s8          vr2, vr3, vr4
    vmul.s16         vr2, vr3, vr4
    vmul.s32         vr2, vr3, vr4
    vmul.eu8         vr2, vr3, vr4
    vmul.eu16        vr2, vr3, vr4
    vmul.es8         vr2, vr3, vr4
    vmul.es16        vr2, vr3, vr4
    vmula.u8         vr2, vr3, vr4
    vmula.u16        vr2, vr3, vr4
    vmula.u32        vr2, vr3, vr4
    vmula.s8         vr2, vr3, vr4
    vmula.s16        vr2, vr3, vr4
    vmula.s32        vr2, vr3, vr4
    vmula.eu8        vr2, vr3, vr4
    vmula.eu16       vr2, vr3, vr4
    vmula.eu32       vr2, vr3, vr4
    vmula.es8        vr2, vr3, vr4
    vmula.es16       vr2, vr3, vr4
    vmula.es32       vr2, vr3, vr4
    vmuls.u8         vr2, vr3, vr4
    vmuls.u16        vr2, vr3, vr4
    vmuls.u32        vr2, vr3, vr4
    vmuls.s8         vr2, vr3, vr4
    vmuls.s16        vr2, vr3, vr4
    vmuls.s32        vr2, vr3, vr4
    vmuls.eu8        vr2, vr3, vr4
    vmuls.eu16       vr2, vr3, vr4
    vmuls.es8        vr2, vr3, vr4
    vmuls.es16       vr2, vr3, vr4
    vshr.u8          vr2, vr3, vr4
    vshr.u16         vr2, vr3, vr4
    vshr.u32         vr2, vr3, vr4
    vshr.s8          vr2, vr3, vr4
    vshr.s16         vr2, vr3, vr4
    vshr.s32         vr2, vr3, vr4
    vshr.u8.r        vr2, vr3, vr4
    vshr.u16.r       vr2, vr3, vr4
    vshr.u32.r       vr2, vr3, vr4
    vshr.s8.r        vr2, vr3, vr4
    vshr.s16.r       vr2, vr3, vr4
    vshr.s32.r       vr2, vr3, vr4
    vshl.u8          vr2, vr3, vr4
    vshl.u16         vr2, vr3, vr4
    vshl.u32         vr2, vr3, vr4
    vshl.s8          vr2, vr3, vr4
    vshl.s16         vr2, vr3, vr4
    vshl.s32         vr2, vr3, vr4
    vshl.u8.s        vr2, vr3, vr4
    vshl.u16.s       vr2, vr3, vr4
    vshl.u32.s       vr2, vr3, vr4
    vshl.s8.s        vr2, vr3, vr4
    vshl.s16.s       vr2, vr3, vr4
    vshl.s32.s       vr2, vr3, vr4
    vcmphs.u8        vr2, vr3, vr4
    vcmphs.u16       vr2, vr3, vr4
    vcmphs.u32       vr2, vr3, vr4
    vcmphs.s8        vr2, vr3, vr4
    vcmphs.s16       vr2, vr3, vr4
    vcmphs.s32       vr2, vr3, vr4
    vcmplt.u8        vr2, vr3, vr4
    vcmplt.u16       vr2, vr3, vr4
    vcmplt.u32       vr2, vr3, vr4
    vcmplt.s8        vr2, vr3, vr4
    vcmplt.s16       vr2, vr3, vr4
    vcmplt.s32       vr2, vr3, vr4
    vcmpne.u8        vr2, vr3, vr4
    vcmpne.u16       vr2, vr3, vr4
    vcmpne.u32       vr2, vr3, vr4
    vcmpne.s8        vr2, vr3, vr4
    vcmpne.s16       vr2, vr3, vr4
    vcmpne.s32       vr2, vr3, vr4
    vmax.u8          vr2, vr3, vr4
    vmax.u16         vr2, vr3, vr4
    vmax.u32         vr2, vr3, vr4
    vmax.s8          vr2, vr3, vr4
    vmax.s16         vr2, vr3, vr4
    vmax.s32         vr2, vr3, vr4
    vmin.u8          vr2, vr3, vr4
    vmin.u16         vr2, vr3, vr4
    vmin.u32         vr2, vr3, vr4
    vmin.s8          vr2, vr3, vr4
    vmin.s16         vr2, vr3, vr4
    vmin.s32         vr2, vr3, vr4
    vcmax.u8         vr2, vr3, vr4
    vcmax.u16        vr2, vr3, vr4
    vcmax.u32        vr2, vr3, vr4
    vcmax.s8         vr2, vr3, vr4
    vcmax.s16        vr2, vr3, vr4
    vcmax.s32        vr2, vr3, vr4
    vcmin.u8         vr2, vr3, vr4
    vcmin.u16        vr2, vr3, vr4
    vcmin.u32        vr2, vr3, vr4
    vcmin.s8         vr2, vr3, vr4
    vcmin.s16        vr2, vr3, vr4
    vcmin.s32        vr2, vr3, vr4
    vand.8           vr2, vr3, vr4
    vand.16          vr2, vr3, vr4
    vand.32          vr2, vr3, vr4
    vandn.8          vr2, vr3, vr4
    vandn.16         vr2, vr3, vr4
    vandn.32         vr2, vr3, vr4
    vor.8            vr2, vr3, vr4
    vor.16           vr2, vr3, vr4
    vor.32           vr2, vr3, vr4
    vnor.8           vr2, vr3, vr4
    vnor.16          vr2, vr3, vr4
    vnor.32          vr2, vr3, vr4
    vxor.8           vr2, vr3, vr4
    vxor.16          vr2, vr3, vr4
    vxor.32          vr2, vr3, vr4
    vtst.8           vr2, vr3, vr4
    vtst.16          vr2, vr3, vr4
    vtst.32          vr2, vr3, vr4
    vbpermz.8        vr2, vr3, vr4
    vbpermz.16       vr2, vr3, vr4
    vbpermz.32       vr2, vr3, vr4
    vbperm.8         vr2, vr3, vr4
    vbperm.16        vr2, vr3, vr4
    vbperm.32        vr2, vr3, vr4
    vdch.8           vr2, vr3, vr4
    vdch.16          vr2, vr3, vr4
    vdch.32          vr2, vr3, vr4
    vdcl.8           vr2, vr3, vr4
    vdcl.16          vr2, vr3, vr4
    vdcl.32          vr2, vr3, vr4
    vich.8           vr2, vr3, vr4
    vich.16          vr2, vr3, vr4
    vich.32          vr2, vr3, vr4
    vicl.8           vr2, vr3, vr4
    vicl.16          vr2, vr3, vr4
    vicl.32          vr2, vr3, vr4
