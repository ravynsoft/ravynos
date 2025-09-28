# Check ADX instructions.
        .allow_index_reg
	.text
_start:
        adcx    400(%ecx), %eax
        adcx    %edx, %ecx
        adcx    -654321(%esp,%esi,8), %edx
        adcx    (%eax), %eax
        adcxl    %edx, %ecx
        adcxl    (%eax), %eax

        adox    400(%ecx), %eax
        adox    %edx, %ecx
        adox    -654321(%esp,%esi,8), %edx
        adox    (%eax), %eax
        adoxl   %edx, %ecx
        adoxl   (%eax), %eax

	.intel_syntax noprefix
	.rept 2

        adcx    eax, DWORD PTR [edx+36]
        adcx    edx, ecx
        adcx    edx, DWORD PTR [esp+esi*8-12]
        adcx    eax, DWORD PTR [eax]

        adox    eax, DWORD PTR [edx+36]
        adox    edx, ecx
        adox    edx, DWORD PTR [esp+esi*8-12]
        adox    eax, DWORD PTR [eax]

	.code16
	.endr
