# 64bit lockable Instructions

	.text
foo:
	lock add %eax, (%rbx)
	lock addl $0x64, (%rbx)
	lock adc %eax, (%rbx)
	lock adcl $0x64, (%rbx)
	lock and %eax, (%rbx)
	lock andl $0x64, (%rbx)
	lock btc %eax, (%rbx)
	lock btcl $0x64, (%rbx)
	lock btr %eax, (%rbx)
	lock btrl $0x64, (%rbx)
	lock bts %eax, (%rbx)
	lock btsl $0x64, (%rbx)
	lock cmpxchg %eax,(%rbx)
	lock cmpxchg8b (%rbx)
	lock cmpxchg16b (%rbx)
	lock decl (%rbx)
	lock incl (%rbx)
	lock negl (%rbx)
	lock notl (%rbx)
	lock or %eax, (%rbx)
	lock orl $0x64, (%rbx)
	lock sbb %eax, (%rbx)
	lock sbbl $0x64, (%rbx)
	lock sub %eax, (%rbx)
	lock subl $0x64, (%rbx)
	lock xadd %eax, (%rbx)
	lock xchg (%rbx), %eax
	lock xchg %eax, (%rbx)
	lock xor %eax, (%rbx)
	lock xorl $0x64, (%rbx)

	.intel_syntax noprefix
	lock add DWORD PTR [rbx],eax
	lock add DWORD PTR [rbx],0x64
	lock adc DWORD PTR [rbx],eax
	lock adc DWORD PTR [rbx],0x64
	lock and DWORD PTR [rbx],eax
	lock and DWORD PTR [rbx],0x64
	lock btc DWORD PTR [rbx],eax
	lock btc DWORD PTR [rbx],0x64
	lock btr DWORD PTR [rbx],eax
	lock btr DWORD PTR [rbx],0x64
	lock bts DWORD PTR [rbx],eax
	lock bts DWORD PTR [rbx],0x64
	lock cmpxchg DWORD PTR [rbx],eax
	lock cmpxchg8b QWORD PTR [rbx]
	lock cmpxchg16b OWORD PTR [rbx]
	lock dec DWORD PTR [rbx]
	lock inc DWORD PTR [rbx]
	lock neg DWORD PTR [rbx]
	lock not DWORD PTR [rbx]
	lock or DWORD PTR [rbx],eax
	lock or DWORD PTR [rbx],0x64
	lock sbb DWORD PTR [rbx],eax
	lock sbb DWORD PTR [rbx],0x64
	lock sub DWORD PTR [rbx],eax
	lock sub DWORD PTR [rbx],0x64
	lock xadd DWORD PTR [rbx],eax
	lock xchg DWORD PTR [rbx],eax
	lock xchg DWORD PTR [rbx],eax
	lock xor DWORD PTR [rbx],eax
	lock xor DWORD PTR [rbx],0x64
