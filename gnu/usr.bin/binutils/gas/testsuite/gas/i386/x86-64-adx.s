# Check 64 bit ADX instructions.
        .allow_index_reg
	.text
_start:
	adcx    400(%ecx), %eax
	adcx    %edx, %ecx
	adcx    -654321(%esp,%esi,8), %edx
	adcx    (%eax), %eax
	adcxl    %edx, %ecx
	adcxl    (%eax), %eax

	adcx    400(%rcx), %r11
	adcx    %r14, %r12
	adcx    -654321(%esp,%esi,8), %rdx
	adcx    (%r8), %rax
	adcxq    %rdx, %rcx
	adcxq    (%rax), %rax

	adox    400(%ecx), %eax
	adox    %edx, %ecx
	adox    -654321(%esp,%esi,8), %edx
	adox    (%eax), %eax
	adoxl    %edx, %ecx
	adoxl    (%eax), %eax

	adox    400(%rcx), %r11
	adox    %r14, %r12
	adox    -654321(%esp,%esi,8), %rdx
	adox    (%r8), %rax
	adoxq    %rdx, %rcx
	adoxq    (%rax), %rax

	.intel_syntax noprefix

	adcx    eax, DWORD PTR [edx+399]
	adcx    edx, ecx
	adcx    edx, DWORD PTR [esp+esi*8-123456]
	adcx    eax, DWORD PTR [eax]

	adcx    rax, QWORD PTR [r11+399]
	adcx    rdx, r9
	adcx    rdx, QWORD PTR [rsp+rsi*8-123456]
	adcx    rax, [rbx]

	adox    eax, DWORD PTR [edx+399]
	adox    edx, ecx
	adox    edx, DWORD PTR [esp+esi*8-123456]
	adox    eax, DWORD PTR [eax]

	adox    rax, QWORD PTR [r11+399]
	adox    rdx, r9
	adox    rdx, QWORD PTR [rsp+rsi*8-123456]
	adox    rax, QWORD PTR [rbx]
