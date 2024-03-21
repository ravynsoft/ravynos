# Check vgather instructions

	.text
vgather:
	vgatherdps %xmm2,(%eax,%xmm1,1),%xmm0
	vgatherdps %xmm2,(%eax,%xmm1,2),%xmm2
	vgatherdps %xmm1,(%eax,%xmm1,4),%xmm0
	vgatherdps %xmm2,(%eax,%xmm1,8),%xmm1

avx512vgather:
	vgatherdpd	123(%ebp,%ymm7,8), %zmm6{%k1}
	vgatherdpd	123(%ebp,%ymm6,8), %zmm6{%k1}
	vgatherdps	123(%ebp,%zmm7,8), %zmm6{%k1}
	vgatherdps	123(%ebp,%zmm6,8), %zmm6{%k1}
	vgatherqpd	123(%ebp,%zmm7,8), %zmm6{%k1}
	vgatherqpd	123(%ebp,%zmm6,8), %zmm6{%k1}
	vgatherqps	123(%ebp,%zmm7,8), %ymm6{%k1}
	vgatherqps	123(%ebp,%zmm6,8), %ymm6{%k1}
	vpgatherdd	123(%ebp,%zmm7,8), %zmm6{%k1}
	vpgatherdd	123(%ebp,%zmm6,8), %zmm6{%k1}
	vpgatherdq	123(%ebp,%ymm7,8), %zmm6{%k1}
	vpgatherdq	123(%ebp,%ymm6,8), %zmm6{%k1}
	vpgatherqd	123(%ebp,%zmm7,8), %ymm6{%k1}
	vpgatherqd	123(%ebp,%zmm6,8), %ymm6{%k1}
	vpgatherqq	123(%ebp,%zmm7,8), %zmm6{%k1}
	vpgatherqq	123(%ebp,%zmm6,8), %zmm6{%k1}
	vpgatherqd	123(%ebp,%ymm7,8), %xmm6{%k1}
	vpgatherqd	123(%ebp,%ymm6,8), %xmm6{%k1}
