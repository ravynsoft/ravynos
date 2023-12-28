.text

L0:
    fabsm    fr0, fr2, r4
    fnegm    fr0, fr2, r4
    faddm   fr0, fr2, fr4, r4
    fsubm   fr0, fr2, fr4, r4
    fmacm   fr0, fr2, fr4, r4
    fmscm   fr0, fr2, fr4, r4
    fmulm   fr0, fr2, fr4, r4
    fnmacm   fr0, fr2, fr4, r4
    fnmscm   fr0, fr2, fr4, r4
    fnmulm   fr0, fr2, fr4, r4

L1:
    fabss   fr0, fr3, r4
    fnegs   fr0, fr3, r4
    fsqrts  fr0, fr3, r4
    frecips  fr0, fr3, r4
    fadds   fr0, fr2, fr3, r4
    fsubs   fr0, fr2, fr3, r4
    fmacs   fr0, fr2, fr3, r4
    fmscs   fr0, fr2, fr3, r4
    fmuls   fr0, fr2, fr3, r4
    fdivs   fr0, fr2, fr3, r4
    fnmacs   fr0, fr2, fr3, r4
    fnmscs   fr0, fr2, fr3, r4
    fnmuls   fr0, fr2, fr3, r4

    fabsd   fr0, fr2, r4
    fnegd   fr0, fr2, r4
    fsqrtd  fr0, fr2, r4
    frecipd  fr0, fr2, r4
    faddd   fr0, fr2, fr4, r4
    fsubd   fr0, fr2, fr4, r4
    fmacd   fr0, fr2, fr4, r4
    fmscd   fr0, fr2, fr4, r4
    fmuld   fr0, fr2, fr4, r4
    fdivd   fr0, fr2, fr4, r4
    fnmacd   fr0, fr2, fr4, r4
    fnmscd   fr0, fr2, fr4, r4
    fnmuld   fr0, fr2, fr4, r4

L2:
    fcmphsd fr0, fr2, r4
    fcmpltd fr0, fr2, r4
    fcmpned fr0, fr2, r4
    fcmpuod fr0, fr2, r4
    fcmphss fr1, fr2, r4
    fcmplts fr1, fr2, r4
    fcmpnes fr1, fr2, r4
    fcmpuos fr1, fr2, r4
    fcmpzhsd fr0, r3
    fcmpzltd fr0, r3
    fcmpzned fr0, r3
    fcmpzuod fr0, r3
    fcmpzhss fr0, r3
    fcmpzlts fr0, r3
    fcmpznes fr0, r3
    fcmpzuos fr0, r3

L3:
    fstod    fr0, fr1, r2
    fdtos    fr0, fr2, r2
    fsitos   fr0, fr2, r2
    fsitod   fr0, fr2, r2
    fuitos   fr0, fr2, r2
    fuitod   fr0, fr2, r2
    fstosi   fr0, fr2, RM_NEAREST, r3
    fdtosi   fr0, fr2, RM_NEAREST, r3
    fstoui   fr0, fr2, RM_NEAREST, r3
    fdtoui   fr0, fr2, RM_NEAREST, r3

L4:
    fmts     r2, fr4
    fmfs     r2, fr4
    fmtd     r2, fr4
    fmfd     r2, fr4
