
.text
hello:
.loop_start:
    ldbi.b r2, (r3)
    ldbi.h r2, (r3)
    ldbi.w r2, (r3)
.loop_end:
    bloop r2, .loop_start, .loop_end
    bloop r2, .loop_start
    pldbi.d r2, (r3)
    ldbi.hs r2, (r3)
    ldbi.bs r2, (r3)
    stbi.b r2, (r3)
    stbi.h r2, (r3)
    stbi.w r2, (r3)
    ldbir.b r2, (r3), r4
    ldbir.h r2, (r3), r4
    ldbir.w r2, (r3), r4
    pldbir.d r2, (r3), r4
    ldbir.hs r2, (r3), r4
    ldbir.bs r2, (r3), r4
    stbir.b r2, (r3), r4
    stbir.h r2, (r3), r4
    stbir.w r2, (r3), r4

    padd.8  r2, r3, r4
    padd.16  r2, r3, r4
    padd.u8.s  r2, r3, r4
    padd.s8.s  r2, r3, r4
    padd.u16.s  r2, r3, r4
    padd.s16.s  r2, r3, r4
    add.u32.s  r2, r3, r4
    add.s32.s  r2, r3, r4
    psub.8  r2, r3, r4
    psub.16  r2, r3, r4
    psub.u8.s  r2, r3, r4
    psub.s8.s  r2, r3, r4
    psub.u16.s  r2, r3, r4
    psub.s16.s  r2, r3, r4
    sub.u32.s  r2, r3, r4
    sub.s32.s  r2, r3, r4
    paddh.u8   r2, r3, r4
    paddh.s8   r2, r3, r4
    paddh.u16   r2, r3, r4
    paddh.s16   r2, r3, r4
    addh.u32   r2, r3, r4
    addh.s32   r2, r3, r4
    psubh.u8   r2, r3, r4
    psubh.s8   r2, r3, r4
    psubh.u16   r2, r3, r4
    psubh.s16   r2, r3, r4
    subh.u32   r2, r3, r4
    subh.s32   r2, r3, r4

    pasx.16    r2, r3, r4
    psax.16    r2, r3, r4
    pasx.s16.s    r2, r3, r4
    pasx.u16.s    r2, r3, r4
    psax.u16.s    r2, r3, r4
    psax.s16.s    r2, r3, r4
    pasxh.u16    r2, r3, r4
    pasxh.s16    r2, r3, r4
    psaxh.u16    r2, r3, r4
    psaxh.s16    r2, r3, r4
    pcmpne.8    r2, r3, r4
    pcmpne.16    r2, r3, r4
    pcmphs.u8    r2, r3, r4
    pcmphs.s16    r2, r3, r4
    pcmphs.u16    r2, r3, r4
    pcmphs.s8    r2, r3, r4
    pcmplt.u8    r2, r3, r4
    pcmplt.u16    r2, r3, r4
    pcmplt.s16    r2, r3, r4
    pcmplt.u16    r2, r3, r4
    pmax.u8    r2, r3, r4
    pmax.s8    r2, r3, r4
    pmax.u16    r2, r3, r4
    pmax.s16    r2, r3, r4
    max.u32    r2, r3, r4
    max.s32    r2, r3, r4
    pmin.u8    r2, r3, r4
    pmin.s8    r2, r3, r4
    pmin.u16    r2, r3, r4
    pmin.s16    r2, r3, r4
    min.u32    r2, r3, r4
    min.s32    r2, r3, r4
    sel   r3, r2, r1, r4

    psabsa.u8  r2, r3, r4
    psabsaa.u8  r2, r3, r4
    divul       r2, r3, r4
    divsl       r2, r3, r4
    mulaca.s8   r2, r3, r4

    asri.s32.r   r2, r3, 16
    asr.s32.r   r2, r3, r6
    lsri.u32.r  r2, r3, 16
    lsr.u32.r   r2, r3, r6
    lsli.u32.s   r2, r3, 16
    lsli.s32.s   r2, r3, 16
    lsl.u32.s    r2, r3, r6
    lsl.s32.s    r2, r3, r6
    pasri.s16   r2, r3, 8
    pasr.s16    r2, r3, r6
    pasri.s16.r  r2, r3, 8
    pasr.s16.r   r2, r3, r6
    plsri.u16    r2, r3, 8
    plsr.u16     r2, r3, r4
    plsri.u16.r  r2, r3, 8
    plsr.u16.r   r2, r3, r4
    plsli.16    r2, r3, 8
    plsl.16     r2, r3, r16
    plsli.u16.s  r2, r3, 8
    plsli.s16.s  r2, r3, 8
    plsl.u16.s   r2, r3, r4
    plsl.s16.s   r2, r3, r4

    pkg   r2, r3, 4, r5, 3
    dexti  r2, r3, r4, 4
    dext  r2, r3, r4, r5
    pkgll r2, r3, r4
    pkghh r2, r3, r4
    pext.u8.e r2, r3
    pext.s8.e r2, r3
    pextx.u8.e r2, r3
    pextx.s8.e r2, r3
    narl r2, r3, r4
    narh r2, r3, r4
    narlx r2, r3, r4
    narhx r2, r3, r4
    clipi.u32 r2, r3, 16
    clipi.s32 r2, r3, 16
    clip.u32 r2, r3, r4
    clip.s32 r2, r3, r4
    pclipi.s16 r2, r3, 4
    pclipi.u16 r2, r3, 4
    pclip.s16 r2, r3, r4
    pclip.u16 r2, r3, r4
    pabs.s8.s  r2, r3
    pabs.s16.s  r2, r3
    abs.s32.s  r2, r3
    pneg.s8.s  r2, r3
    pneg.s16.s  r2, r3
    neg.s32.s  r2, r3
    dup.8  r2, r3, 3
    dup.16  r2, r3, 0

    mul.u32  r2, r3, r4
    mul.s32  r2, r3, r4
    mula.u32 r2, r3, r4
    mula.s32 r2, r3, r4
    muls.u32 r2, r3, r4
    muls.s32 r2, r3, r4
    mula.u32.s r2, r3, r4
    mula.s32.s r2, r3, r4
    muls.u32.s r2, r3, r4
    muls.s32.s r2, r3, r4
    mul.s32.h  r2, r3, r4
    mul.s32.rh  r2, r3, r4
    rmul.s32.h  r2, r3, r4
    rmul.s32.rh  r2, r3, r4
    mula.s32.hs r2, r3, r4
    muls.s32.hs  r2, r3, r4
    mula.s32.rhs r2, r3, r4
    muls.s32.rhs  r2, r3, r4
    mulxl.s32 r2, r3, r4
    mulxl.s32.r  r2, r3, r4
    mulxh.s32 r2, r3, r4
    mulxh.s32.r r2, r3, r4
    rmulxl.s32  r2, r3, r4
    rmulxl.s32.r r2, r3, r4
    rmulxh.s32 r2, r3, r4
    rmulxh.s32.r r2, r3, r4
    mulaxl.s32.s  r2, r3, r4
    mulaxl.s32.rs r2, r3, r4
    mulaxh.s32.s r2, r3, r4
    mulaxh.s32.rs r2, r3, r4
    mulll.s16 r2, r3, r4
    mulhh.s16 r2, r3, r4
    mulhl.s16 r2, r3, r4
    rmulll.s16 r2, r3, r4
    rmulhh.s16 r2, r3, r4
    rmulhl.s16 r2, r3, r4
    mulall.s16.s r2, r3, r4
    mulahh.s16.s r2, r3, r4
    mulahl.s16.s r2, r3, r4
    mulall.s16.e  r2, r3, r4
    mulahh.s16.e r2, r3, r4
    mulahl.s16.e r2, r3, r4
    pmul.u16 r2, r3, r4
    pmulx.u16 r2, r3, r4
    pmul.s16 r2, r3, r4
    pmulx.s16 r2, r3, r4
    prmul.s16 r2, r3, r4
    prmulx.s16 r2, r3, r4
    prmul.s16.h r2, r3, r4
    prmul.s16.rh r2, r3, r4
    prmulx.s16.h r2, r3, r4
    prmulx.s16.rh r2, r3, r4
    mulca.s16.s r2, r3, r4
    mulcax.s16.s r2, r3, r4
    mulcs.s16 r2, r3, r4
    mulcsr.s16 r2, r3, r4
    mulcsx.s16 r2, r3, r4
    mulaca.s16.s r2, r3, r4
    mulacax.s16.s r2, r3, r4
    mulacs.s16.s r2, r3, r4
    mulacsr.s16.s r2, r3, r4
    mulacsx.s16.s r2, r3, r4
    mulsca.s16.s r2, r3, r4
    mulscax.s16.s r2, r3, r4
    mulaca.s16.e r2, r3, r4
    mulacax.s16.e r2, r3, r4
    mulacs.s16.e r2, r3, r4
    mulacsr.s16.e r2, r3, r4
    mulacsx.s16.e r2, r3, r4
    mulsca.s16.e r2, r3, r4
    mulsca.s16.e r2, r3, r4
    mula.32.l   r2, r3, r4
