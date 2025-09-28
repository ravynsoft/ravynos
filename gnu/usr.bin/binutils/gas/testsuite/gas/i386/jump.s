.psize 0
.text
.extern xxx
.extern yyy

1:	jmp	1b
	jmp	xxx
	jmp	*xxx
	jmp	*%edi
	jmp	*(%edi)
	ljmp	*xxx(,%edi,4)
	ljmpw	*xxx(,%edi,4)
	ljmp	*xxx
	ljmpw	*xxx
	ljmp	$0x1234,$xxx

	call	1b
	call	xxx
	call	*xxx
	call	*%edi
	call	*(%edi)
	lcall	*xxx(,%edi,4)
	lcallw	*xxx(,%edi,4)
	lcall	*xxx
	lcallw	*xxx
	lcall	$0x1234,$xxx

	.intel_syntax noprefix
	call	word ptr [ebx]
	call	dword ptr [ebx]
	call	fword ptr [ebx]
	jmp	word ptr [ebx]
	jmp	dword ptr [ebx]
	jmp	fword ptr [ebx]
	jmp	$+2
	nop
	jecxz	2+$
	nop
	jmp	.+2
	nop

	lcall	0x9090,0x90909090
	lcall	0x9090:0x90909090
	lcall	0x9090,xxx
	lcall	0x9090:xxx
	call	0x9090,0x90909090
	call	0x9090:0x90909090
	call	0x9090,xxx
	call	0x9090:xxx
	ljmp	0x9090,0x90909090
	ljmp	0x9090:0x90909090
	ljmp	0x9090,xxx
	ljmp	0x9090:xxx
	jmp	0x9090,0x90909090
	jmp	0x9090:0x90909090
	jmp	0x9090,xxx
	jmp	0x9090:xxx
	ljmp	yyy,0x90909090
	ljmp	yyy:0x90909090
	ljmp	yyy,xxx
	ljmp	yyy:xxx
	jmp	yyy,0x90909090
	jmp	yyy:0x90909090
	jmp	yyy,xxx
	jmp	yyy:xxx
