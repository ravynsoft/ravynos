	.text
start:
	.code32
	.insn 0x0f22, (%edi), %cr3
	.insn 0x0f20, %cr3, (%edi)
	.insn 0x0f21, %db3, (%edi)
	.insn 0x0f23, (%edi), %db3
