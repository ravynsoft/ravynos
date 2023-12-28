#as: -J
#objdump: -dwMintel
#name: i386 intel
#source: intel.s
#warning_output: intel.e

.*: +file format .*

Disassembly of section .text:

0+000 <foo>:
[ 	]*[a-f0-9]+:	00 90 90 90 90 90 +	add    BYTE PTR \[eax-0x6f6f6f70\],dl
[ 	]*[a-f0-9]+:	01 90 90 90 90 90 +	add    DWORD PTR \[eax-0x6f6f6f70\],edx
[ 	]*[a-f0-9]+:	02 90 90 90 90 90 +	add    dl,BYTE PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	03 90 90 90 90 90 +	add    edx,DWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	04 90 +	add    al,0x90
[ 	]*[a-f0-9]+:	05 90 90 90 90 +	add    eax,0x90909090
[ 	]*[a-f0-9]+:	06 +	push   es
[ 	]*[a-f0-9]+:	07 +	pop    es
[ 	]*[a-f0-9]+:	08 90 90 90 90 90 +	or     BYTE PTR \[eax-0x6f6f6f70\],dl
[ 	]*[a-f0-9]+:	09 90 90 90 90 90 +	or     DWORD PTR \[eax-0x6f6f6f70\],edx
[ 	]*[a-f0-9]+:	0a 90 90 90 90 90 +	or     dl,BYTE PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0b 90 90 90 90 90 +	or     edx,DWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0c 90 +	or     al,0x90
[ 	]*[a-f0-9]+:	0d 90 90 90 90 +	or     eax,0x90909090
[ 	]*[a-f0-9]+:	0e +	push   cs
[ 	]*[a-f0-9]+:	10 90 90 90 90 90 +	adc    BYTE PTR \[eax-0x6f6f6f70\],dl
[ 	]*[a-f0-9]+:	11 90 90 90 90 90 +	adc    DWORD PTR \[eax-0x6f6f6f70\],edx
[ 	]*[a-f0-9]+:	12 90 90 90 90 90 +	adc    dl,BYTE PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	13 90 90 90 90 90 +	adc    edx,DWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	14 90 +	adc    al,0x90
[ 	]*[a-f0-9]+:	15 90 90 90 90 +	adc    eax,0x90909090
[ 	]*[a-f0-9]+:	16 +	push   ss
[ 	]*[a-f0-9]+:	17 +	pop    ss
[ 	]*[a-f0-9]+:	18 90 90 90 90 90 +	sbb    BYTE PTR \[eax-0x6f6f6f70\],dl
[ 	]*[a-f0-9]+:	19 90 90 90 90 90 +	sbb    DWORD PTR \[eax-0x6f6f6f70\],edx
[ 	]*[a-f0-9]+:	1a 90 90 90 90 90 +	sbb    dl,BYTE PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	1b 90 90 90 90 90 +	sbb    edx,DWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	1c 90 +	sbb    al,0x90
[ 	]*[a-f0-9]+:	1d 90 90 90 90 +	sbb    eax,0x90909090
[ 	]*[a-f0-9]+:	1e +	push   ds
[ 	]*[a-f0-9]+:	1f +	pop    ds
[ 	]*[a-f0-9]+:	20 90 90 90 90 90 +	and    BYTE PTR \[eax-0x6f6f6f70\],dl
[ 	]*[a-f0-9]+:	21 90 90 90 90 90 +	and    DWORD PTR \[eax-0x6f6f6f70\],edx
[ 	]*[a-f0-9]+:	22 90 90 90 90 90 +	and    dl,BYTE PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	23 90 90 90 90 90 +	and    edx,DWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	24 90 +	and    al,0x90
[ 	]*[a-f0-9]+:	25 90 90 90 90 +	and    eax,0x90909090
[ 	]*[a-f0-9]+:	27 +	daa
[ 	]*[a-f0-9]+:	28 90 90 90 90 90 +	sub    BYTE PTR \[eax-0x6f6f6f70\],dl
[ 	]*[a-f0-9]+:	29 90 90 90 90 90 +	sub    DWORD PTR \[eax-0x6f6f6f70\],edx
[ 	]*[a-f0-9]+:	2a 90 90 90 90 90 +	sub    dl,BYTE PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	2b 90 90 90 90 90 +	sub    edx,DWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	2c 90 +	sub    al,0x90
[ 	]*[a-f0-9]+:	2d 90 90 90 90 +	sub    eax,0x90909090
[ 	]*[a-f0-9]+:	2f +	das
[ 	]*[a-f0-9]+:	30 90 90 90 90 90 +	xor    BYTE PTR \[eax-0x6f6f6f70\],dl
[ 	]*[a-f0-9]+:	31 90 90 90 90 90 +	xor    DWORD PTR \[eax-0x6f6f6f70\],edx
[ 	]*[a-f0-9]+:	32 90 90 90 90 90 +	xor    dl,BYTE PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	33 90 90 90 90 90 +	xor    edx,DWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	34 90 +	xor    al,0x90
[ 	]*[a-f0-9]+:	35 90 90 90 90 +	xor    eax,0x90909090
[ 	]*[a-f0-9]+:	37 +	aaa
[ 	]*[a-f0-9]+:	38 90 90 90 90 90 +	cmp    BYTE PTR \[eax-0x6f6f6f70\],dl
[ 	]*[a-f0-9]+:	39 90 90 90 90 90 +	cmp    DWORD PTR \[eax-0x6f6f6f70\],edx
[ 	]*[a-f0-9]+:	3a 90 90 90 90 90 +	cmp    dl,BYTE PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	3b 90 90 90 90 90 +	cmp    edx,DWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	3c 90 +	cmp    al,0x90
[ 	]*[a-f0-9]+:	3d 90 90 90 90 +	cmp    eax,0x90909090
[ 	]*[a-f0-9]+:	3f +	aas
[ 	]*[a-f0-9]+:	40 +	inc    eax
[ 	]*[a-f0-9]+:	41 +	inc    ecx
[ 	]*[a-f0-9]+:	42 +	inc    edx
[ 	]*[a-f0-9]+:	43 +	inc    ebx
[ 	]*[a-f0-9]+:	44 +	inc    esp
[ 	]*[a-f0-9]+:	45 +	inc    ebp
[ 	]*[a-f0-9]+:	46 +	inc    esi
[ 	]*[a-f0-9]+:	47 +	inc    edi
[ 	]*[a-f0-9]+:	48 +	dec    eax
[ 	]*[a-f0-9]+:	49 +	dec    ecx
[ 	]*[a-f0-9]+:	4a +	dec    edx
[ 	]*[a-f0-9]+:	4b +	dec    ebx
[ 	]*[a-f0-9]+:	4c +	dec    esp
[ 	]*[a-f0-9]+:	4d +	dec    ebp
[ 	]*[a-f0-9]+:	4e +	dec    esi
[ 	]*[a-f0-9]+:	4f +	dec    edi
[ 	]*[a-f0-9]+:	50 +	push   eax
[ 	]*[a-f0-9]+:	51 +	push   ecx
[ 	]*[a-f0-9]+:	52 +	push   edx
[ 	]*[a-f0-9]+:	53 +	push   ebx
[ 	]*[a-f0-9]+:	54 +	push   esp
[ 	]*[a-f0-9]+:	55 +	push   ebp
[ 	]*[a-f0-9]+:	56 +	push   esi
[ 	]*[a-f0-9]+:	57 +	push   edi
[ 	]*[a-f0-9]+:	58 +	pop    eax
[ 	]*[a-f0-9]+:	59 +	pop    ecx
[ 	]*[a-f0-9]+:	5a +	pop    edx
[ 	]*[a-f0-9]+:	5b +	pop    ebx
[ 	]*[a-f0-9]+:	5c +	pop    esp
[ 	]*[a-f0-9]+:	5d +	pop    ebp
[ 	]*[a-f0-9]+:	5e +	pop    esi
[ 	]*[a-f0-9]+:	5f +	pop    edi
[ 	]*[a-f0-9]+:	60 +	pusha
[ 	]*[a-f0-9]+:	61 +	popa
[ 	]*[a-f0-9]+:	62 90 90 90 90 90 +	bound  edx,QWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	63 90 90 90 90 90 +	arpl   WORD PTR \[eax-0x6f6f6f70\],dx
[ 	]*[a-f0-9]+:	68 90 90 90 90 +	push   0x90909090
[ 	]*[a-f0-9]+:	69 90 90 90 90 90 90 90 90 90 	imul   edx,DWORD PTR \[eax-0x6f6f6f70\],0x90909090
[ 	]*[a-f0-9]+:	6a 90 +	push   0xffffff90
[ 	]*[a-f0-9]+:	6b 90 90 90 90 90 90 	imul   edx,DWORD PTR \[eax-0x6f6f6f70\],0xffffff90
[ 	]*[a-f0-9]+:	6c +	ins    BYTE PTR es:\[edi\],dx
[ 	]*[a-f0-9]+:	6d +	ins    DWORD PTR es:\[edi\],dx
[ 	]*[a-f0-9]+:	6e +	outs   dx,BYTE PTR ds:\[esi\]
[ 	]*[a-f0-9]+:	6f +	outs   dx,DWORD PTR ds:\[esi\]
[ 	]*[a-f0-9]+:	70 90 +	jo     df <foo\+0xdf>
[ 	]*[a-f0-9]+:	71 90 +	jno    e1 <foo\+0xe1>
[ 	]*[a-f0-9]+:	72 90 +	jb     e3 <foo\+0xe3>
[ 	]*[a-f0-9]+:	73 90 +	jae    e5 <foo\+0xe5>
[ 	]*[a-f0-9]+:	74 90 +	je     e7 <foo\+0xe7>
[ 	]*[a-f0-9]+:	75 90 +	jne    e9 <foo\+0xe9>
[ 	]*[a-f0-9]+:	76 90 +	jbe    eb <foo\+0xeb>
[ 	]*[a-f0-9]+:	77 90 +	ja     ed <foo\+0xed>
[ 	]*[a-f0-9]+:	78 90 +	js     ef <foo\+0xef>
[ 	]*[a-f0-9]+:	79 90 +	jns    f1 <foo\+0xf1>
[ 	]*[a-f0-9]+:	7a 90 +	jp     f3 <foo\+0xf3>
[ 	]*[a-f0-9]+:	7b 90 +	jnp    f5 <foo\+0xf5>
[ 	]*[a-f0-9]+:	7c 90 +	jl     f7 <foo\+0xf7>
[ 	]*[a-f0-9]+:	7d 90 +	jge    f9 <foo\+0xf9>
[ 	]*[a-f0-9]+:	7e 90 +	jle    fb <foo\+0xfb>
[ 	]*[a-f0-9]+:	7f 90 +	jg     fd <foo\+0xfd>
[ 	]*[a-f0-9]+:	80 90 90 90 90 90 90 	adc    BYTE PTR \[eax-0x6f6f6f70\],0x90
[ 	]*[a-f0-9]+:	81 90 90 90 90 90 90 90 90 90 	adc    DWORD PTR \[eax-0x6f6f6f70\],0x90909090
[ 	]*[a-f0-9]+:	83 90 90 90 90 90 90 	adc    DWORD PTR \[eax-0x6f6f6f70\],0xffffff90
[ 	]*[a-f0-9]+:	84 90 90 90 90 90 +	test   BYTE PTR \[eax-0x6f6f6f70\],dl
[ 	]*[a-f0-9]+:	85 90 90 90 90 90 +	test   DWORD PTR \[eax-0x6f6f6f70\],edx
[ 	]*[a-f0-9]+:	86 90 90 90 90 90 +	xchg   BYTE PTR \[eax-0x6f6f6f70\],dl
[ 	]*[a-f0-9]+:	87 90 90 90 90 90 +	xchg   DWORD PTR \[eax-0x6f6f6f70\],edx
[ 	]*[a-f0-9]+:	88 90 90 90 90 90 +	mov    BYTE PTR \[eax-0x6f6f6f70\],dl
[ 	]*[a-f0-9]+:	89 90 90 90 90 90 +	mov    DWORD PTR \[eax-0x6f6f6f70\],edx
[ 	]*[a-f0-9]+:	8a 90 90 90 90 90 +	mov    dl,BYTE PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	8b 90 90 90 90 90 +	mov    edx,DWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	8c 90 90 90 90 90 +	mov    WORD PTR \[eax-0x6f6f6f70\],ss
[ 	]*[a-f0-9]+:	8d 90 90 90 90 90 +	lea    edx,\[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	8e 90 90 90 90 90 +	mov    ss,WORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	8f 80 90 90 90 90 +	pop    DWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	90 +	nop
[ 	]*[a-f0-9]+:	91 +	xchg   ecx,eax
[ 	]*[a-f0-9]+:	92 +	xchg   edx,eax
[ 	]*[a-f0-9]+:	93 +	xchg   ebx,eax
[ 	]*[a-f0-9]+:	94 +	xchg   esp,eax
[ 	]*[a-f0-9]+:	95 +	xchg   ebp,eax
[ 	]*[a-f0-9]+:	96 +	xchg   esi,eax
[ 	]*[a-f0-9]+:	97 +	xchg   edi,eax
[ 	]*[a-f0-9]+:	98 +	cwde
[ 	]*[a-f0-9]+:	99 +	cdq
[ 	]*[a-f0-9]+:	9a 90 90 90 90 90 90 	call   0x9090:0x90909090
[ 	]*[a-f0-9]+:	9b +	fwait
[ 	]*[a-f0-9]+:	9c +	pushf
[ 	]*[a-f0-9]+:	9d +	popf
[ 	]*[a-f0-9]+:	9e +	sahf
[ 	]*[a-f0-9]+:	9f +	lahf
[ 	]*[a-f0-9]+:	a0 90 90 90 90 +	mov    al,ds:0x90909090
[ 	]*[a-f0-9]+:	a1 90 90 90 90 +	mov    eax,ds:0x90909090
[ 	]*[a-f0-9]+:	a2 90 90 90 90 +	mov    ds:0x90909090,al
[ 	]*[a-f0-9]+:	a3 90 90 90 90 +	mov    ds:0x90909090,eax
[ 	]*[a-f0-9]+:	a4 +	movs   BYTE PTR es:\[edi\],BYTE PTR ds:\[esi\]
[ 	]*[a-f0-9]+:	a5 +	movs   DWORD PTR es:\[edi\],DWORD PTR ds:\[esi\]
[ 	]*[a-f0-9]+:	a6 +	cmps   BYTE PTR ds:\[esi\],BYTE PTR es:\[edi\]
[ 	]*[a-f0-9]+:	a7 +	cmps   DWORD PTR ds:\[esi\],DWORD PTR es:\[edi\]
[ 	]*[a-f0-9]+:	a8 90 +	test   al,0x90
[ 	]*[a-f0-9]+:	a9 90 90 90 90 +	test   eax,0x90909090
[ 	]*[a-f0-9]+:	aa +	stos   BYTE PTR es:\[edi\],al
[ 	]*[a-f0-9]+:	ab +	stos   DWORD PTR es:\[edi\],eax
[ 	]*[a-f0-9]+:	ac +	lods   al,BYTE PTR ds:\[esi\]
[ 	]*[a-f0-9]+:	ad +	lods   eax,DWORD PTR ds:\[esi\]
[ 	]*[a-f0-9]+:	ae +	scas   al,BYTE PTR es:\[edi\]
[ 	]*[a-f0-9]+:	af +	scas   eax,DWORD PTR es:\[edi\]
[ 	]*[a-f0-9]+:	b0 90 +	mov    al,0x90
[ 	]*[a-f0-9]+:	b1 90 +	mov    cl,0x90
[ 	]*[a-f0-9]+:	b2 90 +	mov    dl,0x90
[ 	]*[a-f0-9]+:	b3 90 +	mov    bl,0x90
[ 	]*[a-f0-9]+:	b4 90 +	mov    ah,0x90
[ 	]*[a-f0-9]+:	b5 90 +	mov    ch,0x90
[ 	]*[a-f0-9]+:	b6 90 +	mov    dh,0x90
[ 	]*[a-f0-9]+:	b7 90 +	mov    bh,0x90
[ 	]*[a-f0-9]+:	b8 90 90 90 90 +	mov    eax,0x90909090
[ 	]*[a-f0-9]+:	b9 90 90 90 90 +	mov    ecx,0x90909090
[ 	]*[a-f0-9]+:	ba 90 90 90 90 +	mov    edx,0x90909090
[ 	]*[a-f0-9]+:	bb 90 90 90 90 +	mov    ebx,0x90909090
[ 	]*[a-f0-9]+:	bc 90 90 90 90 +	mov    esp,0x90909090
[ 	]*[a-f0-9]+:	bd 90 90 90 90 +	mov    ebp,0x90909090
[ 	]*[a-f0-9]+:	be 90 90 90 90 +	mov    esi,0x90909090
[ 	]*[a-f0-9]+:	bf 90 90 90 90 +	mov    edi,0x90909090
[ 	]*[a-f0-9]+:	c0 90 90 90 90 90 90 	rcl    BYTE PTR \[eax-0x6f6f6f70\],0x90
[ 	]*[a-f0-9]+:	c1 90 90 90 90 90 90 	rcl    DWORD PTR \[eax-0x6f6f6f70\],0x90
[ 	]*[a-f0-9]+:	c2 90 90 +	ret    0x9090
[ 	]*[a-f0-9]+:	c3 +	ret
[ 	]*[a-f0-9]+:	c4 90 90 90 90 90 +	les    edx,FWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	c5 90 90 90 90 90 +	lds    edx,FWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	c6 80 90 90 90 90 90 	mov    BYTE PTR \[eax-0x6f6f6f70\],0x90
[ 	]*[a-f0-9]+:	c7 80 90 90 90 90 90 90 90 90 	mov    DWORD PTR \[eax-0x6f6f6f70\],0x90909090
[ 	]*[a-f0-9]+:	c8 90 90 90 +	enter  0x9090,0x90
[ 	]*[a-f0-9]+:	c9 +	leave
[ 	]*[a-f0-9]+:	ca 90 90 +	retf   0x9090
[ 	]*[a-f0-9]+:	cb +	retf
[ 	]*[a-f0-9]+:	ca 90 90 +	retf   0x9090
[ 	]*[a-f0-9]+:	cb +	retf
[ 	]*[a-f0-9]+:	cc +	int3
[ 	]*[a-f0-9]+:	cd 90 +	int    0x90
[ 	]*[a-f0-9]+:	ce +	into
[ 	]*[a-f0-9]+:	cf +	iret
[ 	]*[a-f0-9]+:	d0 90 90 90 90 90 +	rcl    BYTE PTR \[eax-0x6f6f6f70\],1
[ 	]*[a-f0-9]+:	d1 90 90 90 90 90 +	rcl    DWORD PTR \[eax-0x6f6f6f70\],1
[ 	]*[a-f0-9]+:	d2 90 90 90 90 90 +	rcl    BYTE PTR \[eax-0x6f6f6f70\],cl
[ 	]*[a-f0-9]+:	d3 90 90 90 90 90 +	rcl    DWORD PTR \[eax-0x6f6f6f70\],cl
[ 	]*[a-f0-9]+:	d4 90 +	aam    0x90
[ 	]*[a-f0-9]+:	d5 90 +	aad    0x90
[ 	]*[a-f0-9]+:	d7 +	xlat   BYTE PTR ds:\[ebx\]
[ 	]*[a-f0-9]+:	d8 90 90 90 90 90 +	fcom   DWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	d9 90 90 90 90 90 +	fst    DWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	da 90 90 90 90 90 +	ficom  DWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	db 90 90 90 90 90 +	fist   DWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	dc 90 90 90 90 90 +	fcom   QWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	dd 90 90 90 90 90 +	fst    QWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	de 90 90 90 90 90 +	ficom  WORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	df 90 90 90 90 90 +	fist   WORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	e0 90 +	loopne 260 <foo\+0x260>
[ 	]*[a-f0-9]+:	e1 90 +	loope  262 <foo\+0x262>
[ 	]*[a-f0-9]+:	e2 90 +	loop   264 <foo\+0x264>
[ 	]*[a-f0-9]+:	e3 90 +	jecxz  266 <foo\+0x266>
[ 	]*[a-f0-9]+:	e4 90 +	in     al,0x90
[ 	]*[a-f0-9]+:	e5 90 +	in     eax,0x90
[ 	]*[a-f0-9]+:	e6 90 +	out    0x90,al
[ 	]*[a-f0-9]+:	e7 90 +	out    0x90,eax
[ 	]*[a-f0-9]+:	e8 90 90 90 90 +	call   90909... <barn\+0x90908...>
[ 	]*[a-f0-9]+:	e9 90 90 90 90 +	jmp    90909... <barn\+0x90908...>
[ 	]*[a-f0-9]+:	ea 90 90 90 90 90 90 	jmp    0x9090:0x90909090
[ 	]*[a-f0-9]+:	eb 90 +	jmp    281 <foo\+0x281>
[ 	]*[a-f0-9]+:	ec +	in     al,dx
[ 	]*[a-f0-9]+:	ed +	in     eax,dx
[ 	]*[a-f0-9]+:	ee +	out    dx,al
[ 	]*[a-f0-9]+:	ef +	out    dx,eax
[ 	]*[a-f0-9]+:	f4 +	hlt
[ 	]*[a-f0-9]+:	f5 +	cmc
[ 	]*[a-f0-9]+:	f6 90 90 90 90 90 +	not    BYTE PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	f7 90 90 90 90 90 +	not    DWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	f8 +	clc
[ 	]*[a-f0-9]+:	f9 +	stc
[ 	]*[a-f0-9]+:	fa +	cli
[ 	]*[a-f0-9]+:	fb +	sti
[ 	]*[a-f0-9]+:	fc +	cld
[ 	]*[a-f0-9]+:	fd +	std
[ 	]*[a-f0-9]+:	ff 90 90 90 90 90 +	call   DWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f 00 90 90 90 90 90 	lldt   WORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f 01 90 90 90 90 90 	lgdtd  \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f 02 90 90 90 90 90 	lar    edx,WORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f 03 90 90 90 90 90 	lsl    edx,WORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f 06 +	clts
[ 	]*[a-f0-9]+:	0f 08 +	invd
[ 	]*[a-f0-9]+:	0f 09 +	wbinvd
[ 	]*[a-f0-9]+:	0f 0b +	ud2
[ 	]*[a-f0-9]+:	0f 20 d0 +	mov    eax,cr2
[ 	]*[a-f0-9]+:	0f 21 d0 +	mov    eax,dr2
[ 	]*[a-f0-9]+:	0f 22 d0 +	mov    cr2,eax
[ 	]*[a-f0-9]+:	0f 23 d0 +	mov    dr2,eax
[ 	]*[a-f0-9]+:	0f 24 d0 +	mov    eax,tr2
[ 	]*[a-f0-9]+:	0f 26 d0 +	mov    tr2,eax
[ 	]*[a-f0-9]+:	0f 30 +	wrmsr
[ 	]*[a-f0-9]+:	0f 31 +	rdtsc
[ 	]*[a-f0-9]+:	0f 32 +	rdmsr
[ 	]*[a-f0-9]+:	0f 33 +	rdpmc
[ 	]*[a-f0-9]+:	0f 40 90 90 90 90 90 	cmovo  edx,DWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f 41 90 90 90 90 90 	cmovno edx,DWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f 42 90 90 90 90 90 	cmovb  edx,DWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f 43 90 90 90 90 90 	cmovae edx,DWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f 44 90 90 90 90 90 	cmove  edx,DWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f 45 90 90 90 90 90 	cmovne edx,DWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f 46 90 90 90 90 90 	cmovbe edx,DWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f 47 90 90 90 90 90 	cmova  edx,DWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f 48 90 90 90 90 90 	cmovs  edx,DWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f 49 90 90 90 90 90 	cmovns edx,DWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f 4a 90 90 90 90 90 	cmovp  edx,DWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f 4b 90 90 90 90 90 	cmovnp edx,DWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f 4c 90 90 90 90 90 	cmovl  edx,DWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f 4d 90 90 90 90 90 	cmovge edx,DWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f 4e 90 90 90 90 90 	cmovle edx,DWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f 4f 90 90 90 90 90 	cmovg  edx,DWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f 60 90 90 90 90 90 	punpcklbw mm2,DWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f 61 90 90 90 90 90 	punpcklwd mm2,DWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f 62 90 90 90 90 90 	punpckldq mm2,DWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f 63 90 90 90 90 90 	packsswb mm2,QWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f 64 90 90 90 90 90 	pcmpgtb mm2,QWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f 65 90 90 90 90 90 	pcmpgtw mm2,QWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f 66 90 90 90 90 90 	pcmpgtd mm2,QWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f 67 90 90 90 90 90 	packuswb mm2,QWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f 68 90 90 90 90 90 	punpckhbw mm2,QWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f 69 90 90 90 90 90 	punpckhwd mm2,QWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f 6a 90 90 90 90 90 	punpckhdq mm2,QWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f 6b 90 90 90 90 90 	packssdw mm2,QWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f 6e 90 90 90 90 90 	movd   mm2,DWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f 6f 90 90 90 90 90 	movq   mm2,QWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f 71 d0 90 +	psrlw  mm0,0x90
[ 	]*[a-f0-9]+:	0f 72 d0 90 +	psrld  mm0,0x90
[ 	]*[a-f0-9]+:	0f 73 d0 90 +	psrlq  mm0,0x90
[ 	]*[a-f0-9]+:	0f 74 90 90 90 90 90 	pcmpeqb mm2,QWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f 75 90 90 90 90 90 	pcmpeqw mm2,QWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f 76 90 90 90 90 90 	pcmpeqd mm2,QWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f 77 +	emms
[ 	]*[a-f0-9]+:	0f 7e 90 90 90 90 90 	movd   DWORD PTR \[eax-0x6f6f6f70\],mm2
[ 	]*[a-f0-9]+:	0f 7f 90 90 90 90 90 	movq   QWORD PTR \[eax-0x6f6f6f70\],mm2
[ 	]*[a-f0-9]+:	0f 80 90 90 90 90 +	jo     90909... <barn\+0x90908...>
[ 	]*[a-f0-9]+:	0f 81 90 90 90 90 +	jno    90909... <barn\+0x90908...>
[ 	]*[a-f0-9]+:	0f 82 90 90 90 90 +	jb     90909... <barn\+0x90908...>
[ 	]*[a-f0-9]+:	0f 83 90 90 90 90 +	jae    90909... <barn\+0x90908...>
[ 	]*[a-f0-9]+:	0f 84 90 90 90 90 +	je     90909... <barn\+0x90908...>
[ 	]*[a-f0-9]+:	0f 85 90 90 90 90 +	jne    90909... <barn\+0x90908...>
[ 	]*[a-f0-9]+:	0f 86 90 90 90 90 +	jbe    90909... <barn\+0x90908...>
[ 	]*[a-f0-9]+:	0f 87 90 90 90 90 +	ja     90909... <barn\+0x90908...>
[ 	]*[a-f0-9]+:	0f 88 90 90 90 90 +	js     90909... <barn\+0x90908...>
[ 	]*[a-f0-9]+:	0f 89 90 90 90 90 +	jns    90909... <barn\+0x90908...>
[ 	]*[a-f0-9]+:	0f 8a 90 90 90 90 +	jp     90909... <barn\+0x90908...>
[ 	]*[a-f0-9]+:	0f 8b 90 90 90 90 +	jnp    90909... <barn\+0x90908...>
[ 	]*[a-f0-9]+:	0f 8c 90 90 90 90 +	jl     90909... <barn\+0x90908...>
[ 	]*[a-f0-9]+:	0f 8d 90 90 90 90 +	jge    90909... <barn\+0x90908...>
[ 	]*[a-f0-9]+:	0f 8e 90 90 90 90 +	jle    90909... <barn\+0x90908...>
[ 	]*[a-f0-9]+:	0f 8f 90 90 90 90 +	jg     90909... <barn\+0x90908...>
[ 	]*[a-f0-9]+:	0f 90 80 90 90 90 90 	seto   BYTE PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f 91 80 90 90 90 90 	setno  BYTE PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f 92 80 90 90 90 90 	setb   BYTE PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f 93 80 90 90 90 90 	setae  BYTE PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f 94 80 90 90 90 90 	sete   BYTE PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f 95 80 90 90 90 90 	setne  BYTE PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f 96 80 90 90 90 90 	setbe  BYTE PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f 97 80 90 90 90 90 	seta   BYTE PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f 98 80 90 90 90 90 	sets   BYTE PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f 99 80 90 90 90 90 	setns  BYTE PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f 9a 80 90 90 90 90 	setp   BYTE PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f 9b 80 90 90 90 90 	setnp  BYTE PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f 9c 80 90 90 90 90 	setl   BYTE PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f 9d 80 90 90 90 90 	setge  BYTE PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f 9e 80 90 90 90 90 	setle  BYTE PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f 9f 80 90 90 90 90 	setg   BYTE PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f a0 +	push   fs
[ 	]*[a-f0-9]+:	0f a1 +	pop    fs
[ 	]*[a-f0-9]+:	0f a2 +	cpuid
[ 	]*[a-f0-9]+:	0f a3 90 90 90 90 90 	bt     DWORD PTR \[eax-0x6f6f6f70\],edx
[ 	]*[a-f0-9]+:	0f a4 90 90 90 90 90 90 	shld   DWORD PTR \[eax-0x6f6f6f70\],edx,0x90
[ 	]*[a-f0-9]+:	0f a5 90 90 90 90 90 	shld   DWORD PTR \[eax-0x6f6f6f70\],edx,cl
[ 	]*[a-f0-9]+:	0f a8 +	push   gs
[ 	]*[a-f0-9]+:	0f a9 +	pop    gs
[ 	]*[a-f0-9]+:	0f aa +	rsm
[ 	]*[a-f0-9]+:	0f ab 90 90 90 90 90 	bts    DWORD PTR \[eax-0x6f6f6f70\],edx
[ 	]*[a-f0-9]+:	0f ac 90 90 90 90 90 90 	shrd   DWORD PTR \[eax-0x6f6f6f70\],edx,0x90
[ 	]*[a-f0-9]+:	0f ad 90 90 90 90 90 	shrd   DWORD PTR \[eax-0x6f6f6f70\],edx,cl
[ 	]*[a-f0-9]+:	0f af 90 90 90 90 90 	imul   edx,DWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f b0 90 90 90 90 90 	cmpxchg BYTE PTR \[eax-0x6f6f6f70\],dl
[ 	]*[a-f0-9]+:	0f b1 90 90 90 90 90 	cmpxchg DWORD PTR \[eax-0x6f6f6f70\],edx
[ 	]*[a-f0-9]+:	0f b2 90 90 90 90 90 	lss    edx,FWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f b3 90 90 90 90 90 	btr    DWORD PTR \[eax-0x6f6f6f70\],edx
[ 	]*[a-f0-9]+:	0f b4 90 90 90 90 90 	lfs    edx,FWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f b5 90 90 90 90 90 	lgs    edx,FWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f b6 90 90 90 90 90 	movzx  edx,BYTE PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f b7 90 90 90 90 90 	movzx  edx,WORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f 0b +	ud2
[ 	]*[a-f0-9]+:	0f bb 90 90 90 90 90 	btc    DWORD PTR \[eax-0x6f6f6f70\],edx
[ 	]*[a-f0-9]+:	0f bc 90 90 90 90 90 	bsf    edx,DWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f bd 90 90 90 90 90 	bsr    edx,DWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f be 90 90 90 90 90 	movsx  edx,BYTE PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f bf 90 90 90 90 90 	movsx  edx,WORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f c0 90 90 90 90 90 	xadd   BYTE PTR \[eax-0x6f6f6f70\],dl
[ 	]*[a-f0-9]+:	0f c1 90 90 90 90 90 	xadd   DWORD PTR \[eax-0x6f6f6f70\],edx
[ 	]*[a-f0-9]+:	0f c8 +	bswap  eax
[ 	]*[a-f0-9]+:	0f c9 +	bswap  ecx
[ 	]*[a-f0-9]+:	0f ca +	bswap  edx
[ 	]*[a-f0-9]+:	0f cb +	bswap  ebx
[ 	]*[a-f0-9]+:	0f cc +	bswap  esp
[ 	]*[a-f0-9]+:	0f cd +	bswap  ebp
[ 	]*[a-f0-9]+:	0f ce +	bswap  esi
[ 	]*[a-f0-9]+:	0f cf +	bswap  edi
[ 	]*[a-f0-9]+:	0f d1 90 90 90 90 90 	psrlw  mm2,QWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f d2 90 90 90 90 90 	psrld  mm2,QWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f d3 90 90 90 90 90 	psrlq  mm2,QWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f d5 90 90 90 90 90 	pmullw mm2,QWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f d8 90 90 90 90 90 	psubusb mm2,QWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f d9 90 90 90 90 90 	psubusw mm2,QWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f db 90 90 90 90 90 	pand   mm2,QWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f dc 90 90 90 90 90 	paddusb mm2,QWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f dd 90 90 90 90 90 	paddusw mm2,QWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f df 90 90 90 90 90 	pandn  mm2,QWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f e1 90 90 90 90 90 	psraw  mm2,QWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f e2 90 90 90 90 90 	psrad  mm2,QWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f e5 90 90 90 90 90 	pmulhw mm2,QWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f e8 90 90 90 90 90 	psubsb mm2,QWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f e9 90 90 90 90 90 	psubsw mm2,QWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f eb 90 90 90 90 90 	por    mm2,QWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f ec 90 90 90 90 90 	paddsb mm2,QWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f ed 90 90 90 90 90 	paddsw mm2,QWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f ef 90 90 90 90 90 	pxor   mm2,QWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f f1 90 90 90 90 90 	psllw  mm2,QWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f f2 90 90 90 90 90 	pslld  mm2,QWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f f3 90 90 90 90 90 	psllq  mm2,QWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f f5 90 90 90 90 90 	pmaddwd mm2,QWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f f8 90 90 90 90 90 	psubb  mm2,QWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f f9 90 90 90 90 90 	psubw  mm2,QWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f fa 90 90 90 90 90 	psubd  mm2,QWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f fc 90 90 90 90 90 	paddb  mm2,QWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f fd 90 90 90 90 90 	paddw  mm2,QWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f fe 90 90 90 90 90 	paddd  mm2,QWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	66 01 90 90 90 90 90 	add    WORD PTR \[eax-0x6f6f6f70\],dx
[ 	]*[a-f0-9]+:	66 03 90 90 90 90 90 	add    dx,WORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	66 05 90 90 +	add    ax,0x9090
[ 	]*[a-f0-9]+:	66 06 +	pushw  es
[ 	]*[a-f0-9]+:	66 07 +	popw   es
[ 	]*[a-f0-9]+:	66 09 90 90 90 90 90 	or     WORD PTR \[eax-0x6f6f6f70\],dx
[ 	]*[a-f0-9]+:	66 0b 90 90 90 90 90 	or     dx,WORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	66 0d 90 90 +	or     ax,0x9090
[ 	]*[a-f0-9]+:	66 0e +	pushw  cs
[ 	]*[a-f0-9]+:	66 11 90 90 90 90 90 	adc    WORD PTR \[eax-0x6f6f6f70\],dx
[ 	]*[a-f0-9]+:	66 13 90 90 90 90 90 	adc    dx,WORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	66 15 90 90 +	adc    ax,0x9090
[ 	]*[a-f0-9]+:	66 16 +	pushw  ss
[ 	]*[a-f0-9]+:	66 17 +	popw   ss
[ 	]*[a-f0-9]+:	66 19 90 90 90 90 90 	sbb    WORD PTR \[eax-0x6f6f6f70\],dx
[ 	]*[a-f0-9]+:	66 1b 90 90 90 90 90 	sbb    dx,WORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	66 1d 90 90 +	sbb    ax,0x9090
[ 	]*[a-f0-9]+:	66 1e +	pushw  ds
[ 	]*[a-f0-9]+:	66 1f +	popw   ds
[ 	]*[a-f0-9]+:	66 21 90 90 90 90 90 	and    WORD PTR \[eax-0x6f6f6f70\],dx
[ 	]*[a-f0-9]+:	66 23 90 90 90 90 90 	and    dx,WORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	66 25 90 90 +	and    ax,0x9090
[ 	]*[a-f0-9]+:	66 29 90 90 90 90 90 	sub    WORD PTR \[eax-0x6f6f6f70\],dx
[ 	]*[a-f0-9]+:	66 2b 90 90 90 90 90 	sub    dx,WORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	66 2d 90 90 +	sub    ax,0x9090
[ 	]*[a-f0-9]+:	66 31 90 90 90 90 90 	xor    WORD PTR \[eax-0x6f6f6f70\],dx
[ 	]*[a-f0-9]+:	66 33 90 90 90 90 90 	xor    dx,WORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	66 35 90 90 +	xor    ax,0x9090
[ 	]*[a-f0-9]+:	66 39 90 90 90 90 90 	cmp    WORD PTR \[eax-0x6f6f6f70\],dx
[ 	]*[a-f0-9]+:	66 3b 90 90 90 90 90 	cmp    dx,WORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	66 3d 90 90 +	cmp    ax,0x9090
[ 	]*[a-f0-9]+:	66 40 +	inc    ax
[ 	]*[a-f0-9]+:	66 41 +	inc    cx
[ 	]*[a-f0-9]+:	66 42 +	inc    dx
[ 	]*[a-f0-9]+:	66 43 +	inc    bx
[ 	]*[a-f0-9]+:	66 44 +	inc    sp
[ 	]*[a-f0-9]+:	66 45 +	inc    bp
[ 	]*[a-f0-9]+:	66 46 +	inc    si
[ 	]*[a-f0-9]+:	66 47 +	inc    di
[ 	]*[a-f0-9]+:	66 48 +	dec    ax
[ 	]*[a-f0-9]+:	66 49 +	dec    cx
[ 	]*[a-f0-9]+:	66 4a +	dec    dx
[ 	]*[a-f0-9]+:	66 4b +	dec    bx
[ 	]*[a-f0-9]+:	66 4c +	dec    sp
[ 	]*[a-f0-9]+:	66 4d +	dec    bp
[ 	]*[a-f0-9]+:	66 4e +	dec    si
[ 	]*[a-f0-9]+:	66 4f +	dec    di
[ 	]*[a-f0-9]+:	66 50 +	push   ax
[ 	]*[a-f0-9]+:	66 51 +	push   cx
[ 	]*[a-f0-9]+:	66 52 +	push   dx
[ 	]*[a-f0-9]+:	66 53 +	push   bx
[ 	]*[a-f0-9]+:	66 54 +	push   sp
[ 	]*[a-f0-9]+:	66 55 +	push   bp
[ 	]*[a-f0-9]+:	66 56 +	push   si
[ 	]*[a-f0-9]+:	66 57 +	push   di
[ 	]*[a-f0-9]+:	66 58 +	pop    ax
[ 	]*[a-f0-9]+:	66 59 +	pop    cx
[ 	]*[a-f0-9]+:	66 5a +	pop    dx
[ 	]*[a-f0-9]+:	66 5b +	pop    bx
[ 	]*[a-f0-9]+:	66 5c +	pop    sp
[ 	]*[a-f0-9]+:	66 5d +	pop    bp
[ 	]*[a-f0-9]+:	66 5e +	pop    si
[ 	]*[a-f0-9]+:	66 5f +	pop    di
[ 	]*[a-f0-9]+:	66 60 +	pushaw
[ 	]*[a-f0-9]+:	66 61 +	popaw
[ 	]*[a-f0-9]+:	66 62 90 90 90 90 90 	bound  dx,DWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	66 68 90 90 +	pushw  0x9090
[ 	]*[a-f0-9]+:	66 69 90 90 90 90 90 90 90 	imul   dx,WORD PTR \[eax-0x6f6f6f70\],0x9090
[ 	]*[a-f0-9]+:	66 6a 90 +	pushw  0xff90
[ 	]*[a-f0-9]+:	66 6b 90 90 90 90 90 90 	imul   dx,WORD PTR \[eax-0x6f6f6f70\],0xff90
[ 	]*[a-f0-9]+:	66 6d +	ins    WORD PTR es:\[edi\],dx
[ 	]*[a-f0-9]+:	66 6f +	outs   dx,WORD PTR ds:\[esi\]
[ 	]*[a-f0-9]+:	66 81 90 90 90 90 90 90 90 	adc    WORD PTR \[eax-0x6f6f6f70\],0x9090
[ 	]*[a-f0-9]+:	66 83 90 90 90 90 90 90 	adc    WORD PTR \[eax-0x6f6f6f70\],0xff90
[ 	]*[a-f0-9]+:	66 85 90 90 90 90 90 	test   WORD PTR \[eax-0x6f6f6f70\],dx
[ 	]*[a-f0-9]+:	66 87 90 90 90 90 90 	xchg   WORD PTR \[eax-0x6f6f6f70\],dx
[ 	]*[a-f0-9]+:	66 89 90 90 90 90 90 	mov    WORD PTR \[eax-0x6f6f6f70\],dx
[ 	]*[a-f0-9]+:	66 8b 90 90 90 90 90 	mov    dx,WORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	8c 90 90 90 90 90 +	mov    WORD PTR \[eax-0x6f6f6f70\],ss
[ 	]*[a-f0-9]+:	66 8d 90 90 90 90 90 	lea    dx,\[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	66 8f 80 90 90 90 90 	pop    WORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	66 91 +	xchg   cx,ax
[ 	]*[a-f0-9]+:	66 92 +	xchg   dx,ax
[ 	]*[a-f0-9]+:	66 93 +	xchg   bx,ax
[ 	]*[a-f0-9]+:	66 94 +	xchg   sp,ax
[ 	]*[a-f0-9]+:	66 95 +	xchg   bp,ax
[ 	]*[a-f0-9]+:	66 96 +	xchg   si,ax
[ 	]*[a-f0-9]+:	66 97 +	xchg   di,ax
[ 	]*[a-f0-9]+:	66 98 +	cbw
[ 	]*[a-f0-9]+:	66 99 +	cwd
[ 	]*[a-f0-9]+:	66 9a 90 90 90 90 +	call   0x9090:0x9090
[ 	]*[a-f0-9]+:	66 9c +	pushfw
[ 	]*[a-f0-9]+:	66 9d +	popfw
[ 	]*[a-f0-9]+:	66 a1 90 90 90 90 +	mov    ax,ds:0x90909090
[ 	]*[a-f0-9]+:	66 a3 90 90 90 90 +	mov    ds:0x90909090,ax
[ 	]*[a-f0-9]+:	66 a5 +	movs   WORD PTR es:\[edi\],WORD PTR ds:\[esi\]
[ 	]*[a-f0-9]+:	66 a7 +	cmps   WORD PTR ds:\[esi\],WORD PTR es:\[edi\]
[ 	]*[a-f0-9]+:	66 a9 90 90 +	test   ax,0x9090
[ 	]*[a-f0-9]+:	66 ab +	stos   WORD PTR es:\[edi\],ax
[ 	]*[a-f0-9]+:	66 ad +	lods   ax,WORD PTR ds:\[esi\]
[ 	]*[a-f0-9]+:	66 af +	scas   ax,WORD PTR es:\[edi\]
[ 	]*[a-f0-9]+:	66 b8 90 90 +	mov    ax,0x9090
[ 	]*[a-f0-9]+:	66 b9 90 90 +	mov    cx,0x9090
[ 	]*[a-f0-9]+:	66 ba 90 90 +	mov    dx,0x9090
[ 	]*[a-f0-9]+:	66 bb 90 90 +	mov    bx,0x9090
[ 	]*[a-f0-9]+:	66 bc 90 90 +	mov    sp,0x9090
[ 	]*[a-f0-9]+:	66 bd 90 90 +	mov    bp,0x9090
[ 	]*[a-f0-9]+:	66 be 90 90 +	mov    si,0x9090
[ 	]*[a-f0-9]+:	66 bf 90 90 +	mov    di,0x9090
[ 	]*[a-f0-9]+:	66 c1 90 90 90 90 90 90 	rcl    WORD PTR \[eax-0x6f6f6f70\],0x90
[ 	]*[a-f0-9]+:	66 c2 90 90 +	retw   0x9090
[ 	]*[a-f0-9]+:	66 c3 +	retw
[ 	]*[a-f0-9]+:	66 c4 90 90 90 90 90 	les    dx,DWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	66 c5 90 90 90 90 90 	lds    dx,DWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	66 c7 80 90 90 90 90 90 90 	mov    WORD PTR \[eax-0x6f6f6f70\],0x9090
[ 	]*[a-f0-9]+:	66 c8 90 90 90 +	enterw 0x9090,0x90
[ 	]*[a-f0-9]+:	66 c9 +	leavew
[ 	]*[a-f0-9]+:	66 ca 90 90 +	retfw  0x9090
[ 	]*[a-f0-9]+:	66 cb +	retfw
[ 	]*[a-f0-9]+:	66 ca 90 90 +	retfw  0x9090
[ 	]*[a-f0-9]+:	66 cb +	retfw
[ 	]*[a-f0-9]+:	66 cf +	iretw
[ 	]*[a-f0-9]+:	66 d1 90 90 90 90 90 	rcl    WORD PTR \[eax-0x6f6f6f70\],1
[ 	]*[a-f0-9]+:	66 d3 90 90 90 90 90 	rcl    WORD PTR \[eax-0x6f6f6f70\],cl
[ 	]*[a-f0-9]+:	66 e5 90 +	in     ax,0x90
[ 	]*[a-f0-9]+:	66 e7 90 +	out    0x90,ax
[ 	]*[a-f0-9]+:	66 e8 8f 90 +	callw  9... <barn\+0x8...>
[ 	]*[a-f0-9]+:	66 ea 90 90 90 90 +	jmp    0x9090:0x9090
[ 	]*[a-f0-9]+:	66 ed +	in     ax,dx
[ 	]*[a-f0-9]+:	66 ef +	out    dx,ax
[ 	]*[a-f0-9]+:	66 f7 90 90 90 90 90 	not    WORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	66 ff 90 90 90 90 90 	call   WORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	66 0f 02 90 90 90 90 90 	lar    dx,WORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	66 0f 03 90 90 90 90 90 	lsl    dx,WORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	66 0f 40 90 90 90 90 90 	cmovo  dx,WORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	66 0f 41 90 90 90 90 90 	cmovno dx,WORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	66 0f 42 90 90 90 90 90 	cmovb  dx,WORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	66 0f 43 90 90 90 90 90 	cmovae dx,WORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	66 0f 44 90 90 90 90 90 	cmove  dx,WORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	66 0f 45 90 90 90 90 90 	cmovne dx,WORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	66 0f 46 90 90 90 90 90 	cmovbe dx,WORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	66 0f 47 90 90 90 90 90 	cmova  dx,WORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	66 0f 48 90 90 90 90 90 	cmovs  dx,WORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	66 0f 49 90 90 90 90 90 	cmovns dx,WORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	66 0f 4a 90 90 90 90 90 	cmovp  dx,WORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	66 0f 4b 90 90 90 90 90 	cmovnp dx,WORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	66 0f 4c 90 90 90 90 90 	cmovl  dx,WORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	66 0f 4d 90 90 90 90 90 	cmovge dx,WORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	66 0f 4e 90 90 90 90 90 	cmovle dx,WORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	66 0f 4f 90 90 90 90 90 	cmovg  dx,WORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	66 0f a0 +	pushw  fs
[ 	]*[a-f0-9]+:	66 0f a1 +	popw   fs
[ 	]*[a-f0-9]+:	66 0f a3 90 90 90 90 90 	bt     WORD PTR \[eax-0x6f6f6f70\],dx
[ 	]*[a-f0-9]+:	66 0f a4 90 90 90 90 90 90 	shld   WORD PTR \[eax-0x6f6f6f70\],dx,0x90
[ 	]*[a-f0-9]+:	66 0f a5 90 90 90 90 90 	shld   WORD PTR \[eax-0x6f6f6f70\],dx,cl
[ 	]*[a-f0-9]+:	66 0f a8 +	pushw  gs
[ 	]*[a-f0-9]+:	66 0f a9 +	popw   gs
[ 	]*[a-f0-9]+:	66 0f ab 90 90 90 90 90 	bts    WORD PTR \[eax-0x6f6f6f70\],dx
[ 	]*[a-f0-9]+:	66 0f ac 90 90 90 90 90 90 	shrd   WORD PTR \[eax-0x6f6f6f70\],dx,0x90
[ 	]*[a-f0-9]+:	66 0f ad 90 90 90 90 90 	shrd   WORD PTR \[eax-0x6f6f6f70\],dx,cl
[ 	]*[a-f0-9]+:	66 0f af 90 90 90 90 90 	imul   dx,WORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	66 0f b1 90 90 90 90 90 	cmpxchg WORD PTR \[eax-0x6f6f6f70\],dx
[ 	]*[a-f0-9]+:	66 0f b2 90 90 90 90 90 	lss    dx,DWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	66 0f b3 90 90 90 90 90 	btr    WORD PTR \[eax-0x6f6f6f70\],dx
[ 	]*[a-f0-9]+:	66 0f b4 90 90 90 90 90 	lfs    dx,DWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	66 0f b5 90 90 90 90 90 	lgs    dx,DWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	66 0f b6 90 90 90 90 90 	movzx  dx,BYTE PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	66 0f bb 90 90 90 90 90 	btc    WORD PTR \[eax-0x6f6f6f70\],dx
[ 	]*[a-f0-9]+:	66 0f bc 90 90 90 90 90 	bsf    dx,WORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	66 0f bd 90 90 90 90 90 	bsr    dx,WORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	66 0f be 90 90 90 90 90 	movsx  dx,BYTE PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	66 0f c1 90 90 90 90 90 	xadd   WORD PTR \[eax-0x6f6f6f70\],dx

[a-f0-9]+ <gs_foo>:
[ 	]*[a-f0-9]+:	c3 +	ret

[a-f0-9]+ <short_foo>:
[ 	]*[a-f0-9]+:	c3 +	ret

[a-f0-9]+ <bar>:
[ 	]*[a-f0-9]+:	e8 f9 ff ff ff +	call   9d9 <gs_foo>
[ 	]*[a-f0-9]+:	e8 f5 ff ff ff +	call   9da <short_foo>
[ 	]*[a-f0-9]+:	dd 1c d0 +	fstp   QWORD PTR \[eax\+edx\*8\]
[ 	]*[a-f0-9]+:	b9 00 00 00 00 +	mov    ecx,0x0
[ 	]*[a-f0-9]+:	88 04 16 +	mov    BYTE PTR \[esi\+edx\*1\],al
[ 	]*[a-f0-9]+:	88 04 32 +	mov    BYTE PTR \[edx\+esi\*1\],al
[ 	]*[a-f0-9]+:	88 04 56 +	mov    BYTE PTR \[esi\+edx\*2\],al
[ 	]*[a-f0-9]+:	88 04 56 +	mov    BYTE PTR \[esi\+edx\*2\],al
[ 	]*[a-f0-9]+:	eb 0c +	jmp    a07 <rot5>
[ 	]*[a-f0-9]+:	6c +	ins    BYTE PTR es:\[edi\],dx
[ 	]*[a-f0-9]+:	66 0f c1 90 90 90 90 90 	xadd   WORD PTR \[eax-0x6f6f6f70\],dx
[ 	]*[a-f0-9]+:	83 e0 f8 +	and    eax,0xfffffff8

[a-f0-9]+ <rot5>:
[ 	]*[a-f0-9]+:	8b 44 ce 04 +	mov    eax,DWORD PTR \[esi\+ecx\*8\+0x4\]
[ 	]*[a-f0-9]+:	6c +	ins    BYTE PTR es:\[edi\],dx
[ 	]*[a-f0-9]+:	0c 90 +	or     al,0x90
[ 	]*[a-f0-9]+:	0d 90 90 90 90 +	or     eax,0x90909090
[ 	]*[a-f0-9]+:	0e +	push   cs
[ 	]*[a-f0-9]+:	8b 04 5d 00 00 00 00 	mov    eax,DWORD PTR \[ebx\*2\+0x0\]
[ 	]*[a-f0-9]+:	10 14 85 90 90 90 90 	adc    BYTE PTR \[eax\*4-0x6f6f6f70\],dl
[ 	]*[a-f0-9]+:	2f +	das
[ 	]*[a-f0-9]+:	ea 90 90 90 90 90 90 	jmp    0x9090:0x90909090
[ 	]*[a-f0-9]+:	66 a5 +	movs   WORD PTR es:\[edi\],WORD PTR ds:\[esi\]
[ 	]*[a-f0-9]+:	70 90 +	jo     9be <foo\+0x9be>
[ 	]*[a-f0-9]+:	75 fe +	jne    a2e <rot5\+0x27>
[ 	]*[a-f0-9]+:	0f 6f 35 28 00 00 00 	movq   mm6,QWORD PTR ds:0x28
[ 	]*[a-f0-9]+:	03 3c c3 +	add    edi,DWORD PTR \[ebx\+eax\*8\]
[ 	]*[a-f0-9]+:	0f 6e 44 c3 04 +	movd   mm0,DWORD PTR \[ebx\+eax\*8\+0x4\]
[ 	]*[a-f0-9]+:	03 bc cb 00 80 00 00 	add    edi,DWORD PTR \[ebx\+ecx\*8\+0x8000\]
[ 	]*[a-f0-9]+:	0f 6e 8c cb 04 80 00 00 	movd   mm1,DWORD PTR \[ebx\+ecx\*8\+0x8004\]
[ 	]*[a-f0-9]+:	0f 6e 94 c3 04 00 01 00 	movd   mm2,DWORD PTR \[ebx\+eax\*8\+0x10004\]
[ 	]*[a-f0-9]+:	03 bc c3 00 00 01 00 	add    edi,DWORD PTR \[ebx\+eax\*8\+0x10000\]
[ 	]*[a-f0-9]+:	66 8b 04 43 +	mov    ax,WORD PTR \[ebx\+eax\*2\]
[ 	]*[a-f0-9]+:	66 8b 8c 4b 00 20 00 00 	mov    cx,WORD PTR \[ebx\+ecx\*2\+0x2000\]
[ 	]*[a-f0-9]+:	66 8b 84 43 00 40 00 00 	mov    ax,WORD PTR \[ebx\+eax\*2\+0x4000\]
[ 	]*[a-f0-9]+:	ff e0 +	jmp    eax
[ 	]*[a-f0-9]+:	ff 20 +	jmp    DWORD PTR \[eax\]
[ 	]*[a-f0-9]+:	ff 25 db 09 00 00 +	jmp    DWORD PTR ds:0x9db
[ 	]*[a-f0-9]+:	e9 5b ff ff ff +	jmp    9db <bar>
[ 	]*[a-f0-9]+:	b8 12 00 00 00 +	mov    eax,0x12
[ 	]*[a-f0-9]+:	25 ff ff fb ff +	and    eax,0xfffbffff
[ 	]*[a-f0-9]+:	25 ff ff fb ff +	and    eax,0xfffbffff
[ 	]*[a-f0-9]+:	b0 11 +	mov    al,0x11
[ 	]*[a-f0-9]+:	b0 11 +	mov    al,0x11
[ 	]*[a-f0-9]+:	b3 47 +	mov    bl,0x47
[ 	]*[a-f0-9]+:	b3 47 +	mov    bl,0x47
[ 	]*[a-f0-9]+:	0f ad d0 +	shrd   eax,edx,cl
[ 	]*[a-f0-9]+:	0f a5 d0 +	shld   eax,edx,cl
[ 	]*[a-f0-9]+:	de c1 +	faddp  st\(1\),st
[ 	]*[a-f0-9]+:	d8 c3 +	fadd   st,st\(3\)
[ 	]*[a-f0-9]+:	d8 c3 +	fadd   st,st\(3\)
[ 	]*[a-f0-9]+:	dc c3 +	fadd   st\(3\),st
[ 	]*[a-f0-9]+:	d8 03 +	fadd   DWORD PTR \[ebx\]
[ 	]*[a-f0-9]+:	dc 03 +	fadd   QWORD PTR \[ebx\]
[ 	]*[a-f0-9]+:	de c1 +	faddp  st\(1\),st
[ 	]*[a-f0-9]+:	de c3 +	faddp  st\(3\),st
[ 	]*[a-f0-9]+:	de c3 +	faddp  st\(3\),st
[ 	]*[a-f0-9]+:	de f9 +	fdivp  st\(1\),st
[ 	]*[a-f0-9]+:	d8 f3 +	fdiv   st,st\(3\)
[ 	]*[a-f0-9]+:	d8 f3 +	fdiv   st,st\(3\)
[ 	]*[a-f0-9]+:	dc fb +	fdiv   st\(3\),st
[ 	]*[a-f0-9]+:	d8 33 +	fdiv   DWORD PTR \[ebx\]
[ 	]*[a-f0-9]+:	dc 33 +	fdiv   QWORD PTR \[ebx\]
[ 	]*[a-f0-9]+:	de f9 +	fdivp  st\(1\),st
[ 	]*[a-f0-9]+:	de fb +	fdivp  st\(3\),st
[ 	]*[a-f0-9]+:	de fb +	fdivp  st\(3\),st
[ 	]*[a-f0-9]+:	d8 f3 +	fdiv   st,st\(3\)
[ 	]*[a-f0-9]+:	de f1 +	fdivrp st\(1\),st
[ 	]*[a-f0-9]+:	d8 fb +	fdivr  st,st\(3\)
[ 	]*[a-f0-9]+:	d8 fb +	fdivr  st,st\(3\)
[ 	]*[a-f0-9]+:	dc f3 +	fdivr  st\(3\),st
[ 	]*[a-f0-9]+:	d8 3b +	fdivr  DWORD PTR \[ebx\]
[ 	]*[a-f0-9]+:	dc 3b +	fdivr  QWORD PTR \[ebx\]
[ 	]*[a-f0-9]+:	de f1 +	fdivrp st\(1\),st
[ 	]*[a-f0-9]+:	de f3 +	fdivrp st\(3\),st
[ 	]*[a-f0-9]+:	de f3 +	fdivrp st\(3\),st
[ 	]*[a-f0-9]+:	d8 fb +	fdivr  st,st\(3\)
[ 	]*[a-f0-9]+:	de c9 +	fmulp  st\(1\),st
[ 	]*[a-f0-9]+:	d8 cb +	fmul   st,st\(3\)
[ 	]*[a-f0-9]+:	d8 cb +	fmul   st,st\(3\)
[ 	]*[a-f0-9]+:	dc cb +	fmul   st\(3\),st
[ 	]*[a-f0-9]+:	d8 0b +	fmul   DWORD PTR \[ebx\]
[ 	]*[a-f0-9]+:	dc 0b +	fmul   QWORD PTR \[ebx\]
[ 	]*[a-f0-9]+:	de c9 +	fmulp  st\(1\),st
[ 	]*[a-f0-9]+:	de cb +	fmulp  st\(3\),st
[ 	]*[a-f0-9]+:	de cb +	fmulp  st\(3\),st
[ 	]*[a-f0-9]+:	de e9 +	fsubp  st\(1\),st
[ 	]*[a-f0-9]+:	de e1 +	fsubrp st\(1\),st
[ 	]*[a-f0-9]+:	d8 e3 +	fsub   st,st\(3\)
[ 	]*[a-f0-9]+:	d8 e3 +	fsub   st,st\(3\)
[ 	]*[a-f0-9]+:	dc eb +	fsub   st\(3\),st
[ 	]*[a-f0-9]+:	d8 23 +	fsub   DWORD PTR \[ebx\]
[ 	]*[a-f0-9]+:	dc 23 +	fsub   QWORD PTR \[ebx\]
[ 	]*[a-f0-9]+:	de e9 +	fsubp  st\(1\),st
[ 	]*[a-f0-9]+:	de eb +	fsubp  st\(3\),st
[ 	]*[a-f0-9]+:	d8 e3 +	fsub   st,st\(3\)
[ 	]*[a-f0-9]+:	de eb +	fsubp  st\(3\),st
[ 	]*[a-f0-9]+:	d8 eb +	fsubr  st,st\(3\)
[ 	]*[a-f0-9]+:	d8 eb +	fsubr  st,st\(3\)
[ 	]*[a-f0-9]+:	dc e3 +	fsubr  st\(3\),st
[ 	]*[a-f0-9]+:	d8 2b +	fsubr  DWORD PTR \[ebx\]
[ 	]*[a-f0-9]+:	dc 2b +	fsubr  QWORD PTR \[ebx\]
[ 	]*[a-f0-9]+:	de e1 +	fsubrp st\(1\),st
[ 	]*[a-f0-9]+:	de e3 +	fsubrp st\(3\),st
[ 	]*[a-f0-9]+:	de e3 +	fsubrp st\(3\),st
[ 	]*[a-f0-9]+:	d8 eb +	fsubr  st,st\(3\)
[ 	]*[a-f0-9]+:	de 3b +	fidivr WORD PTR \[ebx\]
[ 	]*[a-f0-9]+:	da 3b +	fidivr DWORD PTR \[ebx\]
[ 	]*[a-f0-9]+:	0f 4a 90 90 90 90 90 	cmovp  edx,DWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f 4b 90 90 90 90 90 	cmovnp edx,DWORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	66 0f 4a 90 90 90 90 90 	cmovp  dx,WORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	66 0f 4b 90 90 90 90 90 	cmovnp dx,WORD PTR \[eax-0x6f6f6f70\]
[ 	]*[a-f0-9]+:	0f 02 c0 +	lar    eax,eax
[ 	]*[a-f0-9]+:	66 0f 02 c0 +	lar    ax,ax
[ 	]*[a-f0-9]+:	0f 02 00 +	lar    eax,WORD PTR \[eax\]
[ 	]*[a-f0-9]+:	66 0f 02 00 +	lar    ax,WORD PTR \[eax\]
[ 	]*[a-f0-9]+:	0f 03 c0 +	lsl    eax,eax
[ 	]*[a-f0-9]+:	66 0f 03 c0 +	lsl    ax,ax
[ 	]*[a-f0-9]+:	0f 03 00 +	lsl    eax,WORD PTR \[eax\]
[ 	]*[a-f0-9]+:	66 0f 03 00 +	lsl    ax,WORD PTR \[eax\]
[ 	]*[a-f0-9]+:	8b 04 04 +	mov    eax,DWORD PTR \[esp\+eax\*1\]
[ 	]*[a-f0-9]+:	8b 04 20 +	mov    eax,DWORD PTR \[eax\+eiz\*1\]
[ 	]*[a-f0-9]+:	c4 e2 69 92 04 08 +	vgatherdps xmm0,DWORD PTR \[eax\+xmm1\*1\],xmm2
[ 	]*[a-f0-9]+:	24 2f +	and    al,0x2f
[ 	]*[a-f0-9]+:	0f +	\.byte 0xf

[a-f0-9]+ <barn>:
[ 	]*[a-f0-9]+:	0f ba e2 03 +	bt     edx,0x3
#pass
