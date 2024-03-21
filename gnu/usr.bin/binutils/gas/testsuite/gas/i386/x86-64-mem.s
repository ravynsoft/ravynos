# Check 64bit instructions with one memory operand

	.text
_start:

sgdt (%rsi)
sidt (%rsi)
lgdt (%rsi)
lidt (%rsi)
invlpg (%rsi)
cmpxchg8b (%rsi)
cmpxchg16b (%rsi)
vmptrld (%rsi)
vmclear (%rsi)
vmxon (%rsi)
vmptrst (%rsi)
fxsave (%rsi)
fxrstor (%rsi)
ldmxcsr (%rsi)
stmxcsr (%rsi)
clflush (%rsi)

.intel_syntax noprefix
sgdt [rsi]
sidt [rsi]
lgdt [rsi]
lidt [rsi]
invlpg [rsi]
cmpxchg8b qword ptr [rsi]
cmpxchg16b oword ptr [rsi]
vmptrld qword ptr [rsi]
vmclear qword ptr [rsi]
vmxon qword ptr [rsi]
vmptrst qword ptr [rsi]
fxsave [rsi]
fxrstor [rsi]
ldmxcsr dword ptr [rsi]
stmxcsr dword ptr [rsi]
clflush byte ptr [rsi]

.p2align 4,0
