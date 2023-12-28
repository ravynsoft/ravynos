	.text
unspec:
	vblendvpd	%xmm0, (%eax), %ymm0, %ymm0
	vblendvpd	%ymm0, (%eax), %xmm0, %xmm0
	vblendvps	%xmm0, (%eax), %ymm0, %ymm0
	vblendvps	%ymm0, (%eax), %xmm0, %xmm0
	vfmaddpd	%xmm0, (%eax), %ymm0, %ymm0
	vfmaddpd	%ymm0, (%eax), %xmm0, %xmm0
	vfmaddps	%xmm0, (%eax), %ymm0, %ymm0
	vfmaddps	%ymm0, (%eax), %xmm0, %xmm0
	vgatherdpd	%xmm0, (%eax,%xmm1), %ymm2
	vgatherdpd	%ymm0, (%eax,%xmm1), %xmm2
	vgatherdps	%xmm0, (%eax,%xmm1), %ymm2
	vgatherdps	%ymm0, (%eax,%ymm1), %xmm2
	vgatherqpd	%xmm0, (%eax,%xmm1), %ymm2
	vgatherqpd	%ymm0, (%eax,%ymm1), %xmm2
	vgatherqps	%xmm0, (%eax,%xmm1), %ymm2
	vgatherqps	%xmm0, (%eax,%ymm1), %ymm2
	vpblendvb	%xmm0, (%eax), %ymm0, %ymm0
	vpblendvb	%ymm0, (%eax), %xmm0, %xmm0
	vpcmov		%xmm0, (%eax), %ymm0, %ymm0
	vpcmov		%ymm0, (%eax), %xmm0, %xmm0
	vpermil2pd	$0, %xmm0, (%eax), %ymm0, %ymm0
	vpermil2pd	$0, %ymm0, (%eax), %xmm0, %xmm0
	vpermil2ps	$0, %xmm0, (%eax), %ymm0, %ymm0
	vpermil2ps	$0, %ymm0, (%eax), %xmm0, %xmm0
	vpgatherdd	%xmm0, (%eax,%xmm1), %ymm2
	vpgatherdd	%ymm0, (%eax,%ymm1), %xmm2
	vpgatherdq	%xmm0, (%eax,%xmm1), %ymm2
	vpgatherdq	%ymm0, (%eax,%xmm1), %xmm2
	vpgatherqd	%xmm0, (%eax,%xmm1), %ymm2
	vpgatherqd	%xmm0, (%eax,%ymm1), %ymm2
	vpgatherqq	%xmm0, (%eax,%xmm1), %ymm2
	vpgatherqq	%ymm0, (%eax,%ymm1), %xmm2

	.intel_syntax noprefix

	vblendvpd	xmm0, xmm0, [eax], ymm0
	vblendvpd	ymm0, ymm0, [eax], xmm0
	vblendvps	xmm0, xmm0, [eax], ymm0
	vblendvps	ymm0, ymm0, [eax], xmm0
	vfmaddpd	xmm0, xmm0, [eax], ymm0
	vfmaddpd	ymm0, ymm0, [eax], xmm0
	vfmaddps	xmm0, xmm0, [eax], ymm0
	vfmaddps	ymm0, ymm0, [eax], xmm0
	vgatherdpd	xmm0, [eax+xmm1], ymm2
	vgatherdpd	ymm0, [eax+xmm1], xmm2
	vgatherdps	xmm0, [eax+xmm1], ymm2
	vgatherdps	ymm0, [eax+ymm1], xmm2
	vgatherqpd	xmm0, [eax+xmm1], ymm2
	vgatherqpd	ymm0, [eax+ymm1], xmm2
	vgatherqps	xmm0, [eax+xmm1], ymm2
	vgatherqps	xmm0, [eax+ymm1], ymm2
	vpblendvb	xmm0, xmm0, [eax], ymm0
	vpblendvb	ymm0, ymm0, [eax], xmm0
	vpcmov		xmm0, xmm0, [eax], ymm0
	vpcmov		ymm0, ymm0, [eax], xmm0
	vpermil2pd	xmm0, xmm0, [eax], ymm0, 0
	vpermil2pd	ymm0, ymm0, [eax], xmm0, 0
	vpermil2ps	xmm0, xmm0, [eax], ymm0, 0
	vpermil2ps	ymm0, ymm0, [eax], xmm0, 0
	vpgatherdd	xmm0, [eax+xmm1], ymm2
	vpgatherdd	ymm0, [eax+ymm1], xmm2
	vpgatherdq	xmm0, [eax+xmm1], ymm2
	vpgatherdq	ymm0, [eax+xmm1], xmm2
	vpgatherqd	xmm0, [eax+xmm1], ymm2
	vpgatherqd	xmm0, [eax+ymm1], ymm2
	vpgatherqq	xmm0, [eax+xmm1], ymm2
	vpgatherqq	ymm0, [eax+ymm1], xmm2
