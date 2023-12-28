	.text
	.intel_syntax noprefix
sg:
	vgatherdpd	xmm2, qword ptr [eax+xmm1], xmm0
	vgatherdpd	xmm2{k7}, qword ptr [eax+xmm1]
	vscatterdpd	qword ptr [eax+xmm1]{k7}, xmm0

	vgatherdpd	xmm2, xmmword ptr [eax+xmm1], xmm0
	vgatherdpd	xmm2{k7}, xmmword ptr [eax+xmm1]
	vscatterdpd	xmmword ptr [eax+xmm1]{k7}, xmm0

	vgatherdps	xmm2, dword ptr [eax+xmm1], xmm0
	vgatherdps	xmm2{k7}, dword ptr [eax+xmm1]
	vscatterdps	dword ptr [eax+xmm1]{k7}, xmm0

	vgatherdps	xmm2, xmmword ptr [eax+xmm1], xmm0
	vgatherdps	xmm2{k7}, xmmword ptr [eax+xmm1]
	vscatterdps	xmmword ptr [eax+xmm1]{k7}, xmm0

	vgatherqpd	xmm2, qword ptr [eax+xmm1], xmm0
	vgatherqpd	xmm2{k7}, qword ptr [eax+xmm1]
	vscatterqpd	qword ptr [eax+xmm1]{k7}, xmm0

	vgatherqpd	xmm2, xmmword ptr [eax+xmm1], xmm0
	vgatherqpd	xmm2{k7}, xmmword ptr [eax+xmm1]
	vscatterqpd	xmmword ptr [eax+xmm1]{k7}, xmm0

	vgatherqps	xmm2, dword ptr [eax+xmm1], xmm0
	vgatherqps	xmm2{k7}, dword ptr [eax+xmm1]
	vscatterqps	dword ptr [eax+xmm1]{k7}, xmm0

	vgatherqps	xmm2, xmmword ptr [eax+xmm1], xmm0
	vgatherqps	xmm2{k7}, xmmword ptr [eax+xmm1]
	vscatterqps	xmmword ptr [eax+xmm1]{k7}, xmm0

	vpgatherdd	xmm2, dword ptr [eax+xmm1], xmm0
	vpgatherdd	xmm2{k7}, dword ptr [eax+xmm1]
	vpscatterdd	dword ptr [eax+xmm1]{k7}, xmm0

	vpgatherdd	xmm2, xmmword ptr [eax+xmm1], xmm0
	vpgatherdd	xmm2{k7}, xmmword ptr [eax+xmm1]
	vpscatterdd	xmmword ptr [eax+xmm1]{k7}, xmm0

	vpgatherdq	xmm2, qword ptr [eax+xmm1], xmm0
	vpgatherdq	xmm2{k7}, qword ptr [eax+xmm1]
	vpscatterdq	qword ptr [eax+xmm1]{k7}, xmm0

	vpgatherdq	xmm2, xmmword ptr [eax+xmm1], xmm0
	vpgatherdq	xmm2{k7}, xmmword ptr [eax+xmm1]
	vpscatterdq	xmmword ptr [eax+xmm1]{k7}, xmm0

	vpgatherqd	xmm2, dword ptr [eax+xmm1], xmm0
	vpgatherqd	xmm2{k7}, dword ptr [eax+xmm1]
	vpscatterqd	dword ptr [eax+xmm1]{k7}, xmm0

	vpgatherqd	xmm2, xmmword ptr [eax+xmm1], xmm0
	vpgatherqd	xmm2{k7}, xmmword ptr [eax+xmm1]
	vpscatterqd	xmmword ptr [eax+xmm1]{k7}, xmm0

	vpgatherqq	xmm2, qword ptr [eax+xmm1], xmm0
	vpgatherqq	xmm2{k7}, qword ptr [eax+xmm1]
	vpscatterqq	qword ptr [eax+xmm1]{k7}, xmm0

	vpgatherqq	xmm2, xmmword ptr [eax+xmm1], xmm0
	vpgatherqq	xmm2{k7}, xmmword ptr [eax+xmm1]
	vpscatterqq	xmmword ptr [eax+xmm1]{k7}, xmm0
