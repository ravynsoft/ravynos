# Lockable Instructions

	.text
foo:
	lock add %eax, (%ebx)
	lock addl $0x64, (%ebx)
	lock adc %eax, (%ebx)
	lock adcl $0x64, (%ebx)
	lock and %eax, (%ebx)
	lock andl $0x64, (%ebx)
	lock btc %eax, (%ebx)
	lock btcl $0x64, (%ebx)
	lock btr %eax, (%ebx)
	lock btrl $0x64, (%ebx)
	lock bts %eax, (%ebx)
	lock btsl $0x64, (%ebx)
	lock cmpxchg %eax,(%ebx)
	lock cmpxchg8b (%ebx)
	lock decl (%ebx)
	lock incl (%ebx)
	lock negl (%ebx)
	lock notl (%ebx)
	lock or %eax, (%ebx)
	lock orl $0x64, (%ebx)
	lock sbb %eax, (%ebx)
	lock sbbl $0x64, (%ebx)
	lock sub %eax, (%ebx)
	lock subl $0x64, (%ebx)
	lock xadd %eax, (%ebx)
	lock xchg (%ebx), %eax
	lock xchg %eax, (%ebx)
	lock xor %eax, (%ebx)
	lock xorl $0x64, (%ebx)

	.intel_syntax noprefix
	lock add DWORD PTR [ebx],eax
	lock add DWORD PTR [ebx],0x64
	lock adc DWORD PTR [ebx],eax
	lock adc DWORD PTR [ebx],0x64
	lock and DWORD PTR [ebx],eax
	lock and DWORD PTR [ebx],0x64
	lock btc DWORD PTR [ebx],eax
	lock btc DWORD PTR [ebx],0x64
	lock btr DWORD PTR [ebx],eax
	lock btr DWORD PTR [ebx],0x64
	lock bts DWORD PTR [ebx],eax
	lock bts DWORD PTR [ebx],0x64
	lock cmpxchg DWORD PTR [ebx],eax
	lock cmpxchg8b QWORD PTR [ebx]
	lock dec DWORD PTR [ebx]
	lock inc DWORD PTR [ebx]
	lock neg DWORD PTR [ebx]
	lock not DWORD PTR [ebx]
	lock or DWORD PTR [ebx],eax
	lock or DWORD PTR [ebx],0x64
	lock sbb DWORD PTR [ebx],eax
	lock sbb DWORD PTR [ebx],0x64
	lock sub DWORD PTR [ebx],eax
	lock sub DWORD PTR [ebx],0x64
	lock xadd DWORD PTR [ebx],eax
	lock xchg DWORD PTR [ebx],eax
	lock xchg DWORD PTR [ebx],eax
	lock xor DWORD PTR [ebx],eax
	lock xor DWORD PTR [ebx],0x64
