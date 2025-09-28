# Check instructions with one memory operand

	.text
_start:

sgdt (%esi)
sidt (%esi)
lgdt (%esi)
lidt (%esi)
invlpg (%esi)
cmpxchg8b (%esi)
vmptrld (%esi)
vmclear (%esi)
vmxon (%esi)
vmptrst (%esi)
fxsave (%esi)
fxrstor (%esi)
ldmxcsr (%esi)
stmxcsr (%esi)
clflush (%esi)

.intel_syntax noprefix
sgdt [esi]
sidt [esi]
lgdt [esi]
lidt [esi]
invlpg [esi]
cmpxchg8b qword ptr [esi]
vmptrld qword ptr [esi]
vmclear qword ptr [esi]
vmxon qword ptr [esi]
vmptrst qword ptr [esi]
fxsave [esi]
fxrstor [esi]
ldmxcsr dword ptr [esi]
stmxcsr dword ptr [esi]
clflush byte ptr [esi]

.p2align 4,0
