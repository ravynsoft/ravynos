# Check 64bit fxsave/frstor instructions.

	.text
foo:
	fxsave (%rax)
	fxsave (%r8)
	fxsave (%r8, %rax)
	fxsave (%rax, %r8)
	fxsave (%r8, %r15)
	fxsave64 (%rax)
	fxsave64 (%r8)
	fxsave64 (%r8, %rax)
	fxsave64 (%rax, %r8)
	fxrstor (%rax)
	fxrstor (%r8)
	fxrstor (%r8, %rax)
	fxrstor (%rax, %r8)
	fxrstor (%r8, %r15)
	fxrstor64 (%rax)
	fxrstor64 (%r8)
	fxrstor64 (%r8, %rax)
	fxrstor64 (%rax, %r8)
	fxrstor64 (%r8, %r15)

	.intel_syntax noprefix
fxsave [rax]
fxsave [r8]
fxsave [r8+rax*1]
fxsave [rax+r8*1]
fxsave [r8+r15*1]
fxsave64 [rax]
fxsave64 [r8]
fxsave64 [r8+rax*1]
fxsave64 [rax+r8*1]
fxrstor [rax]
fxrstor [r8]
fxrstor [r8+rax*1]
fxrstor [rax+r8*1]
fxrstor [r8+r15*1]
fxrstor64 [rax]
fxrstor64 [r8]
fxrstor64 [r8+rax*1]
fxrstor64 [rax+r8*1]
fxrstor64 [r8+r15*1]
