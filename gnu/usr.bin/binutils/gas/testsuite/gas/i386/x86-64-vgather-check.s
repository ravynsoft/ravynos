# Check vgather instructions

	.text
vgather:
	vgatherdps %xmm2,(%rax,%xmm1,1),%xmm0
	vgatherdps %xmm2,(%rax,%xmm1,2),%xmm2
	vgatherdps %xmm2,(%rax,%xmm1,2),%xmm10
	vgatherdps %xmm10,(%rax,%xmm1,2),%xmm10
	vgatherdps %xmm1,(%rax,%xmm1,4),%xmm0
	vgatherdps %xmm9,(%rax,%xmm1,4),%xmm0
	vgatherdps %xmm9,(%rax,%xmm9,4),%xmm0
	vgatherdps %xmm2,(%rax,%xmm1,8),%xmm1
	vgatherdps %xmm2,(%rax,%xmm1,8),%xmm9
	vgatherdps %xmm2,(%rax,%xmm9,8),%xmm9

avx512vgather:
	vgatherdpd	123(%rbp,%ymm17,8), %zmm16{%k1}
	vgatherdpd	123(%rbp,%ymm16,8), %zmm16{%k1}
	vgatherdps	123(%rbp,%zmm17,8), %zmm16{%k1}
	vgatherdps	123(%rbp,%zmm16,8), %zmm16{%k1}
	vgatherqpd	123(%rbp,%zmm17,8), %zmm16{%k1}
	vgatherqpd	123(%rbp,%zmm16,8), %zmm16{%k1}
	vgatherqps	123(%rbp,%zmm17,8), %ymm16{%k1}
	vgatherqps	123(%rbp,%zmm16,8), %ymm16{%k1}
	vpgatherdd	123(%rbp,%zmm17,8), %zmm16{%k1}
	vpgatherdd	123(%rbp,%zmm16,8), %zmm16{%k1}
	vpgatherdq	123(%rbp,%ymm17,8), %zmm16{%k1}
	vpgatherdq	123(%rbp,%ymm16,8), %zmm16{%k1}
	vpgatherqd	123(%rbp,%zmm17,8), %ymm16{%k1}
	vpgatherqd	123(%rbp,%zmm16,8), %ymm16{%k1}
	vpgatherqq	123(%rbp,%zmm17,8), %zmm16{%k1}
	vpgatherqq	123(%rbp,%zmm16,8), %zmm16{%k1}
	vpgatherqd	123(%rbp,%ymm17,8), %xmm16{%k1}
	vpgatherqd	123(%rbp,%ymm16,8), %xmm16{%k1}
