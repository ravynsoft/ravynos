/*
 * Copyright 2019 Red Hat Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */
#ifndef __NVHW_DRF_H__
#define __NVHW_DRF_H__

/* Helpers common to all DRF accessors. */
#define DRF_LO(drf)    (0 ? drf)
#define DRF_HI(drf)    (1 ? drf)
#define DRF_BITS(drf)  (DRF_HI(drf) - DRF_LO(drf) + 1)
#define DRF_MASK(drf)  (~0ULL >> (64 - DRF_BITS(drf)))
#define DRF_SMASK(drf) (DRF_MASK(drf) << DRF_LO(drf))

/* Helpers for DRF-MW accessors. */
#define DRF_MX_MW(drf)      drf
#define DRF_MX(drf)         DRF_MX_##drf
#define DRF_MW(drf)         DRF_MX(drf)
#define DRF_MW_SPANS(o,drf) (DRF_LW_IDX((o),drf) != DRF_HW_IDX((o),drf))
#define DRF_MW_SIZE(o)      (sizeof((o)[0]) * 8)

#define DRF_LW_IDX(o,drf)   (DRF_LO(DRF_MW(drf)) / DRF_MW_SIZE(o))
#define DRF_LW_LO(o,drf)    (DRF_LO(DRF_MW(drf)) % DRF_MW_SIZE(o))
#define DRF_LW_HI(o,drf)    (DRF_MW_SPANS((o),drf) ? (DRF_MW_SIZE(o) - 1) : DRF_HW_HI((o),drf))
#define DRF_LW_BITS(o,drf)  (DRF_LW_HI((o),drf) - DRF_LW_LO((o),drf) + 1)
#define DRF_LW_MASK(o,drf)  (~0ULL >> (64 - DRF_LW_BITS((o),drf)))
#define DRF_LW_SMASK(o,drf) (DRF_LW_MASK((o),drf) << DRF_LW_LO((o),drf))
#define DRF_LW_GET(o,drf)   (((o)[DRF_LW_IDX((o),drf)] >> DRF_LW_LO((o),drf)) & DRF_LW_MASK((o),drf))
#define DRF_LW_VAL(o,drf,v) (((v) & DRF_LW_MASK((o),drf)) << DRF_LW_LO((o),drf))
#define DRF_LW_CLR(o,drf)   ((o)[DRF_LW_IDX((o),drf)] & ~DRF_LW_SMASK((o),drf))
#define DRF_LW_SET(o,drf,v) (DRF_LW_CLR((o),drf) | DRF_LW_VAL((o),drf,(v)))

#define DRF_HW_IDX(o,drf)   (DRF_HI(DRF_MW(drf)) / DRF_MW_SIZE(o))
#define DRF_HW_LO(o,drf)    0
#define DRF_HW_HI(o,drf)    (DRF_HI(DRF_MW(drf)) % DRF_MW_SIZE(o))
#define DRF_HW_BITS(o,drf)  (DRF_HW_HI((o),drf) - DRF_HW_LO((o),drf) + 1)
#define DRF_HW_MASK(o,drf)  (~0ULL >> (64 - DRF_HW_BITS((o),drf)))
#define DRF_HW_SMASK(o,drf) (DRF_HW_MASK((o),drf) << DRF_HW_LO((o),drf))
#define DRF_HW_GET(o,drf)   ((o)[DRF_HW_IDX(o,drf)] & DRF_HW_SMASK((o),drf))
#define DRF_HW_VAL(o,drf,v) (((long long)(v) >> DRF_LW_BITS((o),drf)) & DRF_HW_SMASK((o),drf))
#define DRF_HW_CLR(o,drf)   ((o)[DRF_HW_IDX((o),drf)] & ~DRF_HW_SMASK((o),drf))
#define DRF_HW_SET(o,drf,v) (DRF_HW_CLR((o),drf) | DRF_HW_VAL((o),drf,(v)))

/* DRF accessors. */
#define NVVAL_X(drf,v) (((v) & DRF_MASK(drf)) << DRF_LO(drf))
#define NVVAL_N(X,d,r,f,  v) NVVAL_X(d##_##r##_##f, (v))
#define NVVAL_I(X,d,r,f,i,v) NVVAL_X(d##_##r##_##f(i), (v))
#define NVVAL_(X,_1,_2,_3,_4,_5,IMPL,...) IMPL
#define NVVAL(A...) NVVAL_(X, ##A, NVVAL_I, NVVAL_N)(X, ##A)

#define NVDEF_N(X,d,r,f,  v) NVVAL_X(d##_##r##_##f, d##_##r##_##f##_##v)
#define NVDEF_I(X,d,r,f,i,v) NVVAL_X(d##_##r##_##f(i), d##_##r##_##f##_##v)
#define NVDEF_(X,_1,_2,_3,_4,_5,IMPL,...) IMPL
#define NVDEF(A...) NVDEF_(X, ##A, NVDEF_I, NVDEF_N)(X, ##A)

#define NVVAL_GET_X(o,drf) (((o) >> DRF_LO(drf)) & DRF_MASK(drf))
#define NVVAL_GET_N(X,o,d,r,f  ) NVVAL_GET_X(o, d##_##r##_##f)
#define NVVAL_GET_I(X,o,d,r,f,i) NVVAL_GET_X(o, d##_##r##_##f(i))
#define NVVAL_GET_(X,_1,_2,_3,_4,_5,IMPL,...) IMPL
#define NVVAL_GET(A...) NVVAL_GET_(X, ##A, NVVAL_GET_I, NVVAL_GET_N)(X, ##A)

#define NVVAL_SET_X(o,drf,v) (((o) & ~DRF_SMASK(drf)) | NVVAL_X(drf, (v)))
#define NVVAL_SET_N(X,o,d,r,f,  v) NVVAL_SET_X(o, d##_##r##_##f, (v))
#define NVVAL_SET_I(X,o,d,r,f,i,v) NVVAL_SET_X(o, d##_##r##_##f(i), (v))
#define NVVAL_SET_(X,_1,_2,_3,_4,_5,_6,IMPL,...) IMPL
#define NVVAL_SET(A...) NVVAL_SET_(X, ##A, NVVAL_SET_I, NVVAL_SET_N)(X, ##A)

#define NVDEF_SET_N(X,o,d,r,f,  v)                                             \
	NVVAL_SET_X(o, d##_##r##_##f,    d##_##r##_##f##_##v)
#define NVDEF_SET_I(X,o,d,r,f,i,v)                                             \
	NVVAL_SET_X(o, d##_##r##_##f(i), d##_##r##_##f##_##v)
#define NVDEF_SET_(X,_1,_2,_3,_4,_5,_6,IMPL,...) IMPL
#define NVDEF_SET(A...) NVDEF_SET_(X, ##A, NVDEF_SET_I, NVDEF_SET_N)(X, ##A)

/* DRF-MW accessors. */
#define NVVAL_MW_GET_X(o,drf)                                                  \
	((DRF_MW_SPANS((o),drf) ?                                              \
	  (DRF_HW_GET((o),drf) << DRF_LW_BITS((o),drf)) : 0) | DRF_LW_GET((o),drf))
#define NVVAL_MW_GET_N(X,o,d,r,f  ) NVVAL_MW_GET_X((o), d##_##r##_##f)
#define NVVAL_MW_GET_I(X,o,d,r,f,i) NVVAL_MW_GET_X((o), d##_##r##_##f(i))
#define NVVAL_MW_GET_(X,_1,_2,_3,_4,_5,IMPL,...) IMPL
#define NVVAL_MW_GET(A...) NVVAL_MW_GET_(X, ##A, NVVAL_MW_GET_I, NVVAL_MW_GET_N)(X, ##A)

#define NVVAL_MW_SET_X(o,drf,v) do {                                           \
	(o)[DRF_LW_IDX((o),drf)] = DRF_LW_SET((o),drf,(v));                    \
	if (DRF_MW_SPANS((o),drf))                                             \
		(o)[DRF_HW_IDX((o),drf)] = DRF_HW_SET((o),drf,(v));            \
} while(0)
#define NVVAL_MW_SET_N(X,o,d,r,f,  v) NVVAL_MW_SET_X((o), d##_##r##_##f, (v))
#define NVVAL_MW_SET_I(X,o,d,r,f,i,v) NVVAL_MW_SET_X((o), d##_##r##_##f(i), (v))
#define NVVAL_MW_SET_(X,_1,_2,_3,_4,_5,_6,IMPL,...) IMPL
#define NVVAL_MW_SET(A...)                                                     \
	NVVAL_MW_SET_(X, ##A, NVVAL_MW_SET_I, NVVAL_MW_SET_N)(X, ##A)

#define NVDEF_MW_SET_N(X,o,d,r,f,  v)                                          \
	NVVAL_MW_SET_X(o, d##_##r##_##f,    d##_##r##_##f##_##v)
#define NVDEF_MW_SET_I(X,o,d,r,f,i,v)                                          \
	NVVAL_MW_SET_X(o, d##_##r##_##f(i), d##_##r##_##f##_##v)
#define NVDEF_MW_SET_(X,_1,_2,_3,_4,_5,_6,IMPL,...) IMPL
#define NVDEF_MW_SET(A...)                                                     \
	NVDEF_MW_SET_(X, ##A, NVDEF_MW_SET_I, NVDEF_MW_SET_N)(X, ##A)
#endif
