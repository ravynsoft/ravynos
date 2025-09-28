	.text
_start:
	xor %eax, %eax
140:
	testl %eax, %eax
141:
	.nops -(((144f-143f)-(141b-140b)) > 0)*((144f-143f)-(141b-140b))
142:
	xor %eax, %eax
	.pushsection .altinstr_replacement,"ax"
143:
	mov %eax, %eax
	mov %eax, %eax
	mov %eax, %eax
	jmp foo
144:
	.popsection
	xor %eax, %eax
