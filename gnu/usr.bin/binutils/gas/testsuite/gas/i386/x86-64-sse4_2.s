# Streaming SIMD extensions 4.2 Instructions

	.text
foo:
	crc32		%cl,%ebx
	crc32		%cl,%rbx
	crc32		%cx,%ebx
	crc32		%ecx,%ebx
	crc32		%rcx,%rbx
	crc32b		(%rcx),%ebx
	crc32w		(%rcx),%ebx
	crc32l		(%rcx),%ebx
	crc32q		(%rcx),%rbx
	crc32b		%cl,%ebx
	crc32b		%cl,%rbx
	crc32w		%cx,%ebx
	crc32l		%ecx,%ebx
	crc32q		%rcx,%rbx
	pcmpgtq		(%rcx),%xmm0
	pcmpgtq		%xmm1,%xmm0
	pcmpestri	$0x0,(%rcx),%xmm0
	pcmpestri	$0x0,%xmm1,%xmm0
	pcmpestriq	$0x0,(%rcx),%xmm0
	pcmpestril	$0x0,%xmm1,%xmm0
	pcmpestrm	$0x1,(%rcx),%xmm0
	pcmpestrm	$0x1,%xmm1,%xmm0
	pcmpestrmq	$0x1,(%rcx),%xmm0
	pcmpestrml	$0x1,%xmm1,%xmm0
	pcmpistri	$0x2,(%rcx),%xmm0
	pcmpistri	$0x2,%xmm1,%xmm0
	pcmpistrm	$0x3,(%rcx),%xmm0
	pcmpistrm	$0x3,%xmm1,%xmm0
	popcnt		(%rcx),%bx
	popcnt		(%rcx),%ebx
	popcnt		(%rcx),%rbx
	popcntw		(%rcx),%bx
	popcntl		(%rcx),%ebx
	popcntq		(%rcx),%rbx
	popcnt		%cx,%bx
	popcnt		%ecx,%ebx
	popcnt		%rcx,%rbx
	popcntw		%cx,%bx
	popcntl		%ecx,%ebx
	popcntq		%rcx,%rbx

	.intel_syntax noprefix
	crc32  ebx,cl
	crc32  rbx,cl
	crc32  ebx,cx
	crc32  ebx,ecx
	crc32  rbx,rcx
	crc32  ebx,BYTE PTR [rcx]
	crc32  ebx,WORD PTR [rcx]
	crc32  ebx,DWORD PTR [rcx]
	crc32  rbx,QWORD PTR [rcx]
	crc32  ebx,cl
	crc32  rbx,cl
	crc32  ebx,cx
	crc32  ebx,ecx
	crc32  rbx,rcx
	pcmpgtq xmm0,XMMWORD PTR [rcx]
	pcmpgtq xmm0,xmm1
	pcmpestri xmm0,XMMWORD PTR [rcx],0x0
	pcmpestri xmm0,xmm1,0x0
	pcmpestrm xmm0,XMMWORD PTR [rcx],0x1
	pcmpestrm xmm0,xmm1,0x1
	pcmpistri xmm0,XMMWORD PTR [rcx],0x2
	pcmpistri xmm0,xmm1,0x2
	pcmpistrm xmm0,XMMWORD PTR [rcx],0x3
	pcmpistrm xmm0,xmm1,0x3
	popcnt bx,WORD PTR [rcx]
	popcnt ebx,DWORD PTR [rcx]
	popcnt rbx,QWORD PTR [rcx]
	popcnt bx,WORD PTR [rcx]
	popcnt ebx,DWORD PTR [rcx]
	popcnt rbx,QWORD PTR [rcx]
	popcnt bx,cx
	popcnt ebx,ecx
	popcnt rbx,rcx
	popcnt bx,cx
	popcnt ebx,ecx
	popcnt rbx,rcx

	.p2align	4,0
