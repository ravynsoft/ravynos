	.code32

.macro try opcode:vararg
	.insn 0x0f\opcode/0, (%eax)
	.insn 0x0f\opcode/1, (%eax)
	.insn 0x0f\opcode/2, (%eax)
	.insn 0x0f\opcode/3, (%eax)
	.insn 0x0f\opcode/4, (%eax)
	.insn 0x0f\opcode/5, (%eax)
	.insn 0x0f\opcode/6, (%eax)
	.insn 0x0f\opcode/7, (%eax)
.endm

.text

amd_prefetch:
	try 0d

intel_prefetch:
	try 18
