#as: -J
#objdump: -dw
#name: i386 intel (AT&T disassembly)
#warning_output: intel.e

.*: +file format .*

Disassembly of section .text:

0+000 <foo>:
[ 	]*[a-f0-9]+:	00 90 90 90 90 90 [ 	]*add    %dl,-0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	01 90 90 90 90 90 [ 	]*add    %edx,-0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	02 90 90 90 90 90 [ 	]*add    -0x6f6f6f70\(%eax\),%dl
[ 	]*[a-f0-9]+:	03 90 90 90 90 90 [ 	]*add    -0x6f6f6f70\(%eax\),%edx
[ 	]*[a-f0-9]+:	04 90 [ 	]*add    \$0x90,%al
[ 	]*[a-f0-9]+:	05 90 90 90 90 [ 	]*add    \$0x90909090,%eax
[ 	]*[a-f0-9]+:	06 [ 	]*push   %es
[ 	]*[a-f0-9]+:	07 [ 	]*pop    %es
[ 	]*[a-f0-9]+:	08 90 90 90 90 90 [ 	]*or     %dl,-0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	09 90 90 90 90 90 [ 	]*or     %edx,-0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	0a 90 90 90 90 90 [ 	]*or     -0x6f6f6f70\(%eax\),%dl
[ 	]*[a-f0-9]+:	0b 90 90 90 90 90 [ 	]*or     -0x6f6f6f70\(%eax\),%edx
[ 	]*[a-f0-9]+:	0c 90 [ 	]*or     \$0x90,%al
[ 	]*[a-f0-9]+:	0d 90 90 90 90 [ 	]*or     \$0x90909090,%eax
[ 	]*[a-f0-9]+:	0e [ 	]*push   %cs
[ 	]*[a-f0-9]+:	10 90 90 90 90 90 [ 	]*adc    %dl,-0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	11 90 90 90 90 90 [ 	]*adc    %edx,-0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	12 90 90 90 90 90 [ 	]*adc    -0x6f6f6f70\(%eax\),%dl
[ 	]*[a-f0-9]+:	13 90 90 90 90 90 [ 	]*adc    -0x6f6f6f70\(%eax\),%edx
[ 	]*[a-f0-9]+:	14 90 [ 	]*adc    \$0x90,%al
[ 	]*[a-f0-9]+:	15 90 90 90 90 [ 	]*adc    \$0x90909090,%eax
[ 	]*[a-f0-9]+:	16 [ 	]*push   %ss
[ 	]*[a-f0-9]+:	17 [ 	]*pop    %ss
[ 	]*[a-f0-9]+:	18 90 90 90 90 90 [ 	]*sbb    %dl,-0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	19 90 90 90 90 90 [ 	]*sbb    %edx,-0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	1a 90 90 90 90 90 [ 	]*sbb    -0x6f6f6f70\(%eax\),%dl
[ 	]*[a-f0-9]+:	1b 90 90 90 90 90 [ 	]*sbb    -0x6f6f6f70\(%eax\),%edx
[ 	]*[a-f0-9]+:	1c 90 [ 	]*sbb    \$0x90,%al
[ 	]*[a-f0-9]+:	1d 90 90 90 90 [ 	]*sbb    \$0x90909090,%eax
[ 	]*[a-f0-9]+:	1e [ 	]*push   %ds
[ 	]*[a-f0-9]+:	1f [ 	]*pop    %ds
[ 	]*[a-f0-9]+:	20 90 90 90 90 90 [ 	]*and    %dl,-0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	21 90 90 90 90 90 [ 	]*and    %edx,-0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	22 90 90 90 90 90 [ 	]*and    -0x6f6f6f70\(%eax\),%dl
[ 	]*[a-f0-9]+:	23 90 90 90 90 90 [ 	]*and    -0x6f6f6f70\(%eax\),%edx
[ 	]*[a-f0-9]+:	24 90 [ 	]*and    \$0x90,%al
[ 	]*[a-f0-9]+:	25 90 90 90 90 [ 	]*and    \$0x90909090,%eax
[ 	]*[a-f0-9]+:	27 [ 	]*daa
[ 	]*[a-f0-9]+:	28 90 90 90 90 90 [ 	]*sub    %dl,-0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	29 90 90 90 90 90 [ 	]*sub    %edx,-0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	2a 90 90 90 90 90 [ 	]*sub    -0x6f6f6f70\(%eax\),%dl
[ 	]*[a-f0-9]+:	2b 90 90 90 90 90 [ 	]*sub    -0x6f6f6f70\(%eax\),%edx
[ 	]*[a-f0-9]+:	2c 90 [ 	]*sub    \$0x90,%al
[ 	]*[a-f0-9]+:	2d 90 90 90 90 [ 	]*sub    \$0x90909090,%eax
[ 	]*[a-f0-9]+:	2f [ 	]*das
[ 	]*[a-f0-9]+:	30 90 90 90 90 90 [ 	]*xor    %dl,-0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	31 90 90 90 90 90 [ 	]*xor    %edx,-0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	32 90 90 90 90 90 [ 	]*xor    -0x6f6f6f70\(%eax\),%dl
[ 	]*[a-f0-9]+:	33 90 90 90 90 90 [ 	]*xor    -0x6f6f6f70\(%eax\),%edx
[ 	]*[a-f0-9]+:	34 90 [ 	]*xor    \$0x90,%al
[ 	]*[a-f0-9]+:	35 90 90 90 90 [ 	]*xor    \$0x90909090,%eax
[ 	]*[a-f0-9]+:	37 [ 	]*aaa
[ 	]*[a-f0-9]+:	38 90 90 90 90 90 [ 	]*cmp    %dl,-0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	39 90 90 90 90 90 [ 	]*cmp    %edx,-0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	3a 90 90 90 90 90 [ 	]*cmp    -0x6f6f6f70\(%eax\),%dl
[ 	]*[a-f0-9]+:	3b 90 90 90 90 90 [ 	]*cmp    -0x6f6f6f70\(%eax\),%edx
[ 	]*[a-f0-9]+:	3c 90 [ 	]*cmp    \$0x90,%al
[ 	]*[a-f0-9]+:	3d 90 90 90 90 [ 	]*cmp    \$0x90909090,%eax
[ 	]*[a-f0-9]+:	3f [ 	]*aas
[ 	]*[a-f0-9]+:	40 [ 	]*inc    %eax
[ 	]*[a-f0-9]+:	41 [ 	]*inc    %ecx
[ 	]*[a-f0-9]+:	42 [ 	]*inc    %edx
[ 	]*[a-f0-9]+:	43 [ 	]*inc    %ebx
[ 	]*[a-f0-9]+:	44 [ 	]*inc    %esp
[ 	]*[a-f0-9]+:	45 [ 	]*inc    %ebp
[ 	]*[a-f0-9]+:	46 [ 	]*inc    %esi
[ 	]*[a-f0-9]+:	47 [ 	]*inc    %edi
[ 	]*[a-f0-9]+:	48 [ 	]*dec    %eax
[ 	]*[a-f0-9]+:	49 [ 	]*dec    %ecx
[ 	]*[a-f0-9]+:	4a [ 	]*dec    %edx
[ 	]*[a-f0-9]+:	4b [ 	]*dec    %ebx
[ 	]*[a-f0-9]+:	4c [ 	]*dec    %esp
[ 	]*[a-f0-9]+:	4d [ 	]*dec    %ebp
[ 	]*[a-f0-9]+:	4e [ 	]*dec    %esi
[ 	]*[a-f0-9]+:	4f [ 	]*dec    %edi
[ 	]*[a-f0-9]+:	50 [ 	]*push   %eax
[ 	]*[a-f0-9]+:	51 [ 	]*push   %ecx
[ 	]*[a-f0-9]+:	52 [ 	]*push   %edx
[ 	]*[a-f0-9]+:	53 [ 	]*push   %ebx
[ 	]*[a-f0-9]+:	54 [ 	]*push   %esp
[ 	]*[a-f0-9]+:	55 [ 	]*push   %ebp
[ 	]*[a-f0-9]+:	56 [ 	]*push   %esi
[ 	]*[a-f0-9]+:	57 [ 	]*push   %edi
[ 	]*[a-f0-9]+:	58 [ 	]*pop    %eax
[ 	]*[a-f0-9]+:	59 [ 	]*pop    %ecx
[ 	]*[a-f0-9]+:	5a [ 	]*pop    %edx
[ 	]*[a-f0-9]+:	5b [ 	]*pop    %ebx
[ 	]*[a-f0-9]+:	5c [ 	]*pop    %esp
[ 	]*[a-f0-9]+:	5d [ 	]*pop    %ebp
[ 	]*[a-f0-9]+:	5e [ 	]*pop    %esi
[ 	]*[a-f0-9]+:	5f [ 	]*pop    %edi
[ 	]*[a-f0-9]+:	60 [ 	]*pusha
[ 	]*[a-f0-9]+:	61 [ 	]*popa
[ 	]*[a-f0-9]+:	62 90 90 90 90 90 [ 	]*bound  %edx,-0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	63 90 90 90 90 90 [ 	]*arpl   %dx,-0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	68 90 90 90 90 [ 	]*push   \$0x90909090
[ 	]*[a-f0-9]+:	69 90 90 90 90 90 90 90 90 90 [ 	]*imul   \$0x90909090,-0x6f6f6f70\(%eax\),%edx
[ 	]*[a-f0-9]+:	6a 90 [ 	]*push   \$0xffffff90
[ 	]*[a-f0-9]+:	6b 90 90 90 90 90 90 [ 	]*imul   \$0xffffff90,-0x6f6f6f70\(%eax\),%edx
[ 	]*[a-f0-9]+:	6c [ 	]*insb   \(%dx\),%es:\(%edi\)
[ 	]*[a-f0-9]+:	6d [ 	]*insl   \(%dx\),%es:\(%edi\)
[ 	]*[a-f0-9]+:	6e [ 	]*outsb  %ds:\(%esi\),\(%dx\)
[ 	]*[a-f0-9]+:	6f [ 	]*outsl  %ds:\(%esi\),\(%dx\)
[ 	]*[a-f0-9]+:	70 90 [ 	]*jo     (0x)?df.*
[ 	]*[a-f0-9]+:	71 90 [ 	]*jno    (0x)?e1.*
[ 	]*[a-f0-9]+:	72 90 [ 	]*jb     (0x)?e3.*
[ 	]*[a-f0-9]+:	73 90 [ 	]*jae    (0x)?e5.*
[ 	]*[a-f0-9]+:	74 90 [ 	]*je     (0x)?e7.*
[ 	]*[a-f0-9]+:	75 90 [ 	]*jne    (0x)?e9.*
[ 	]*[a-f0-9]+:	76 90 [ 	]*jbe    (0x)?eb.*
[ 	]*[a-f0-9]+:	77 90 [ 	]*ja     (0x)?ed.*
[ 	]*[a-f0-9]+:	78 90 [ 	]*js     (0x)?ef.*
[ 	]*[a-f0-9]+:	79 90 [ 	]*jns    (0x)?f1.*
[ 	]*[a-f0-9]+:	7a 90 [ 	]*jp     (0x)?f3.*
[ 	]*[a-f0-9]+:	7b 90 [ 	]*jnp    (0x)?f5.*
[ 	]*[a-f0-9]+:	7c 90 [ 	]*jl     (0x)?f7.*
[ 	]*[a-f0-9]+:	7d 90 [ 	]*jge    (0x)?f9.*
[ 	]*[a-f0-9]+:	7e 90 [ 	]*jle    (0x)?fb.*
[ 	]*[a-f0-9]+:	7f 90 [ 	]*jg     (0x)?fd.*
[ 	]*[a-f0-9]+:	80 90 90 90 90 90 90 [ 	]*adcb   \$0x90,-0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	81 90 90 90 90 90 90 90 90 90 [ 	]*adcl   \$0x90909090,-0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	83 90 90 90 90 90 90 [ 	]*adcl   \$0xffffff90,-0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	84 90 90 90 90 90 [ 	]*test   %dl,-0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	85 90 90 90 90 90 [ 	]*test   %edx,-0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	86 90 90 90 90 90 [ 	]*xchg   %dl,-0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	87 90 90 90 90 90 [ 	]*xchg   %edx,-0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	88 90 90 90 90 90 [ 	]*mov    %dl,-0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	89 90 90 90 90 90 [ 	]*mov    %edx,-0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	8a 90 90 90 90 90 [ 	]*mov    -0x6f6f6f70\(%eax\),%dl
[ 	]*[a-f0-9]+:	8b 90 90 90 90 90 [ 	]*mov    -0x6f6f6f70\(%eax\),%edx
[ 	]*[a-f0-9]+:	8c 90 90 90 90 90 [ 	]*mov    %ss,-0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	8d 90 90 90 90 90 [ 	]*lea    -0x6f6f6f70\(%eax\),%edx
[ 	]*[a-f0-9]+:	8e 90 90 90 90 90 [ 	]*mov    -0x6f6f6f70\(%eax\),%ss
[ 	]*[a-f0-9]+:	8f 80 90 90 90 90 [ 	]*pop    -0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	90 [ 	]*nop
[ 	]*[a-f0-9]+:	91 [ 	]*xchg   %eax,%ecx
[ 	]*[a-f0-9]+:	92 [ 	]*xchg   %eax,%edx
[ 	]*[a-f0-9]+:	93 [ 	]*xchg   %eax,%ebx
[ 	]*[a-f0-9]+:	94 [ 	]*xchg   %eax,%esp
[ 	]*[a-f0-9]+:	95 [ 	]*xchg   %eax,%ebp
[ 	]*[a-f0-9]+:	96 [ 	]*xchg   %eax,%esi
[ 	]*[a-f0-9]+:	97 [ 	]*xchg   %eax,%edi
[ 	]*[a-f0-9]+:	98 [ 	]*cwtl
[ 	]*[a-f0-9]+:	99 [ 	]*cltd
[ 	]*[a-f0-9]+:	9a 90 90 90 90 90 90 [ 	]*lcall  \$0x9090,\$0x90909090
[ 	]*[a-f0-9]+:	9b [ 	]*fwait
[ 	]*[a-f0-9]+:	9c [ 	]*pushf
[ 	]*[a-f0-9]+:	9d [ 	]*popf
[ 	]*[a-f0-9]+:	9e [ 	]*sahf
[ 	]*[a-f0-9]+:	9f [ 	]*lahf
[ 	]*[a-f0-9]+:	a0 90 90 90 90 [ 	]*mov    0x90909090,%al
[ 	]*[a-f0-9]+:	a1 90 90 90 90 [ 	]*mov    0x90909090,%eax
[ 	]*[a-f0-9]+:	a2 90 90 90 90 [ 	]*mov    %al,0x90909090
[ 	]*[a-f0-9]+:	a3 90 90 90 90 [ 	]*mov    %eax,0x90909090
[ 	]*[a-f0-9]+:	a4 [ 	]*movsb  %ds:\(%esi\),%es:\(%edi\)
[ 	]*[a-f0-9]+:	a5 [ 	]*movsl  %ds:\(%esi\),%es:\(%edi\)
[ 	]*[a-f0-9]+:	a6 [ 	]*cmpsb  %es:\(%edi\),%ds:\(%esi\)
[ 	]*[a-f0-9]+:	a7 [ 	]*cmpsl  %es:\(%edi\),%ds:\(%esi\)
[ 	]*[a-f0-9]+:	a8 90 [ 	]*test   \$0x90,%al
[ 	]*[a-f0-9]+:	a9 90 90 90 90 [ 	]*test   \$0x90909090,%eax
[ 	]*[a-f0-9]+:	aa [ 	]*stos   %al,%es:\(%edi\)
[ 	]*[a-f0-9]+:	ab [ 	]*stos   %eax,%es:\(%edi\)
[ 	]*[a-f0-9]+:	ac [ 	]*lods   %ds:\(%esi\),%al
[ 	]*[a-f0-9]+:	ad [ 	]*lods   %ds:\(%esi\),%eax
[ 	]*[a-f0-9]+:	ae [ 	]*scas   %es:\(%edi\),%al
[ 	]*[a-f0-9]+:	af [ 	]*scas   %es:\(%edi\),%eax
[ 	]*[a-f0-9]+:	b0 90 [ 	]*mov    \$0x90,%al
[ 	]*[a-f0-9]+:	b1 90 [ 	]*mov    \$0x90,%cl
[ 	]*[a-f0-9]+:	b2 90 [ 	]*mov    \$0x90,%dl
[ 	]*[a-f0-9]+:	b3 90 [ 	]*mov    \$0x90,%bl
[ 	]*[a-f0-9]+:	b4 90 [ 	]*mov    \$0x90,%ah
[ 	]*[a-f0-9]+:	b5 90 [ 	]*mov    \$0x90,%ch
[ 	]*[a-f0-9]+:	b6 90 [ 	]*mov    \$0x90,%dh
[ 	]*[a-f0-9]+:	b7 90 [ 	]*mov    \$0x90,%bh
[ 	]*[a-f0-9]+:	b8 90 90 90 90 [ 	]*mov    \$0x90909090,%eax
[ 	]*[a-f0-9]+:	b9 90 90 90 90 [ 	]*mov    \$0x90909090,%ecx
[ 	]*[a-f0-9]+:	ba 90 90 90 90 [ 	]*mov    \$0x90909090,%edx
[ 	]*[a-f0-9]+:	bb 90 90 90 90 [ 	]*mov    \$0x90909090,%ebx
[ 	]*[a-f0-9]+:	bc 90 90 90 90 [ 	]*mov    \$0x90909090,%esp
[ 	]*[a-f0-9]+:	bd 90 90 90 90 [ 	]*mov    \$0x90909090,%ebp
[ 	]*[a-f0-9]+:	be 90 90 90 90 [ 	]*mov    \$0x90909090,%esi
[ 	]*[a-f0-9]+:	bf 90 90 90 90 [ 	]*mov    \$0x90909090,%edi
[ 	]*[a-f0-9]+:	c0 90 90 90 90 90 90 [ 	]*rclb   \$0x90,-0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	c1 90 90 90 90 90 90 [ 	]*rcll   \$0x90,-0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	c2 90 90 [ 	]*ret    \$0x9090
[ 	]*[a-f0-9]+:	c3 [ 	]*ret
[ 	]*[a-f0-9]+:	c4 90 90 90 90 90 [ 	]*les    -0x6f6f6f70\(%eax\),%edx
[ 	]*[a-f0-9]+:	c5 90 90 90 90 90 [ 	]*lds    -0x6f6f6f70\(%eax\),%edx
[ 	]*[a-f0-9]+:	c6 80 90 90 90 90 90 [ 	]*movb   \$0x90,-0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	c7 80 90 90 90 90 90 90 90 90 [ 	]*movl   \$0x90909090,-0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	c8 90 90 90 [ 	]*enter  \$0x9090,\$0x90
[ 	]*[a-f0-9]+:	c9 [ 	]*leave
[ 	]*[a-f0-9]+:	ca 90 90 [ 	]*lret   \$0x9090
[ 	]*[a-f0-9]+:	cb [ 	]*lret
[ 	]*[a-f0-9]+:	ca 90 90 [ 	]*lret   \$0x9090
[ 	]*[a-f0-9]+:	cb [ 	]*lret
[ 	]*[a-f0-9]+:	cc [ 	]*int3
[ 	]*[a-f0-9]+:	cd 90 [ 	]*int    \$0x90
[ 	]*[a-f0-9]+:	ce [ 	]*into
[ 	]*[a-f0-9]+:	cf [ 	]*iret
[ 	]*[a-f0-9]+:	d0 90 90 90 90 90 [ 	]*rclb   -0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	d1 90 90 90 90 90 [ 	]*rcll   -0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	d2 90 90 90 90 90 [ 	]*rclb   %cl,-0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	d3 90 90 90 90 90 [ 	]*rcll   %cl,-0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	d4 90 [ 	]*aam    \$0x90
[ 	]*[a-f0-9]+:	d5 90 [ 	]*aad    \$0x90
[ 	]*[a-f0-9]+:	d7 [ 	]*xlat   %ds:\(%ebx\)
[ 	]*[a-f0-9]+:	d8 90 90 90 90 90 [ 	]*fcoms  -0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	d9 90 90 90 90 90 [ 	]*fsts   -0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	da 90 90 90 90 90 [ 	]*ficoml -0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	db 90 90 90 90 90 [ 	]*fistl  -0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	dc 90 90 90 90 90 [ 	]*fcoml  -0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	dd 90 90 90 90 90 [ 	]*fstl   -0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	de 90 90 90 90 90 [ 	]*ficoms -0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	df 90 90 90 90 90 [ 	]*fists  -0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	e0 90 [ 	]*loopne (0x)?260.*
[ 	]*[a-f0-9]+:	e1 90 [ 	]*loope  (0x)?262.*
[ 	]*[a-f0-9]+:	e2 90 [ 	]*loop   (0x)?264.*
[ 	]*[a-f0-9]+:	e3 90 [ 	]*jecxz  (0x)?266.*
[ 	]*[a-f0-9]+:	e4 90 [ 	]*in     \$0x90,%al
[ 	]*[a-f0-9]+:	e5 90 [ 	]*in     \$0x90,%eax
[ 	]*[a-f0-9]+:	e6 90 [ 	]*out    %al,\$0x90
[ 	]*[a-f0-9]+:	e7 90 [ 	]*out    %eax,\$0x90
[ 	]*[a-f0-9]+:	e8 90 90 90 90 [ 	]*call   (0x)?90909373.*
[ 	]*[a-f0-9]+:	e9 90 90 90 90 [ 	]*jmp    (0x)?90909378.*
[ 	]*[a-f0-9]+:	ea 90 90 90 90 90 90 [ 	]*ljmp   \$0x9090,\$0x90909090
[ 	]*[a-f0-9]+:	eb 90 [ 	]*jmp    (0x)?281.*
[ 	]*[a-f0-9]+:	ec [ 	]*in     \(%dx\),%al
[ 	]*[a-f0-9]+:	ed [ 	]*in     \(%dx\),%eax
[ 	]*[a-f0-9]+:	ee [ 	]*out    %al,\(%dx\)
[ 	]*[a-f0-9]+:	ef [ 	]*out    %eax,\(%dx\)
[ 	]*[a-f0-9]+:	f4 [ 	]*hlt
[ 	]*[a-f0-9]+:	f5 [ 	]*cmc
[ 	]*[a-f0-9]+:	f6 90 90 90 90 90 [ 	]*notb   -0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	f7 90 90 90 90 90 [ 	]*notl   -0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	f8 [ 	]*clc
[ 	]*[a-f0-9]+:	f9 [ 	]*stc
[ 	]*[a-f0-9]+:	fa [ 	]*cli
[ 	]*[a-f0-9]+:	fb [ 	]*sti
[ 	]*[a-f0-9]+:	fc [ 	]*cld
[ 	]*[a-f0-9]+:	fd [ 	]*std
[ 	]*[a-f0-9]+:	ff 90 90 90 90 90 [ 	]*call   \*-0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	0f 00 90 90 90 90 90 [ 	]*lldt   -0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	0f 01 90 90 90 90 90 [ 	]*lgdtl  -0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	0f 02 90 90 90 90 90 [ 	]*lar    -0x6f6f6f70\(%eax\),%edx
[ 	]*[a-f0-9]+:	0f 03 90 90 90 90 90 [ 	]*lsl    -0x6f6f6f70\(%eax\),%edx
[ 	]*[a-f0-9]+:	0f 06 [ 	]*clts
[ 	]*[a-f0-9]+:	0f 08 [ 	]*invd
[ 	]*[a-f0-9]+:	0f 09 [ 	]*wbinvd
[ 	]*[a-f0-9]+:	0f 0b [ 	]*ud2
[ 	]*[a-f0-9]+:	0f 20 d0 [ 	]*mov    %cr2,%eax
[ 	]*[a-f0-9]+:	0f 21 d0 [ 	]*mov    %db2,%eax
[ 	]*[a-f0-9]+:	0f 22 d0 [ 	]*mov    %eax,%cr2
[ 	]*[a-f0-9]+:	0f 23 d0 [ 	]*mov    %eax,%db2
[ 	]*[a-f0-9]+:	0f 24 d0 [ 	]*mov    %tr2,%eax
[ 	]*[a-f0-9]+:	0f 26 d0 [ 	]*mov    %eax,%tr2
[ 	]*[a-f0-9]+:	0f 30 [ 	]*wrmsr
[ 	]*[a-f0-9]+:	0f 31 [ 	]*rdtsc
[ 	]*[a-f0-9]+:	0f 32 [ 	]*rdmsr
[ 	]*[a-f0-9]+:	0f 33 [ 	]*rdpmc
[ 	]*[a-f0-9]+:	0f 40 90 90 90 90 90 [ 	]*cmovo  -0x6f6f6f70\(%eax\),%edx
[ 	]*[a-f0-9]+:	0f 41 90 90 90 90 90 [ 	]*cmovno -0x6f6f6f70\(%eax\),%edx
[ 	]*[a-f0-9]+:	0f 42 90 90 90 90 90 [ 	]*cmovb  -0x6f6f6f70\(%eax\),%edx
[ 	]*[a-f0-9]+:	0f 43 90 90 90 90 90 [ 	]*cmovae -0x6f6f6f70\(%eax\),%edx
[ 	]*[a-f0-9]+:	0f 44 90 90 90 90 90 [ 	]*cmove  -0x6f6f6f70\(%eax\),%edx
[ 	]*[a-f0-9]+:	0f 45 90 90 90 90 90 [ 	]*cmovne -0x6f6f6f70\(%eax\),%edx
[ 	]*[a-f0-9]+:	0f 46 90 90 90 90 90 [ 	]*cmovbe -0x6f6f6f70\(%eax\),%edx
[ 	]*[a-f0-9]+:	0f 47 90 90 90 90 90 [ 	]*cmova  -0x6f6f6f70\(%eax\),%edx
[ 	]*[a-f0-9]+:	0f 48 90 90 90 90 90 [ 	]*cmovs  -0x6f6f6f70\(%eax\),%edx
[ 	]*[a-f0-9]+:	0f 49 90 90 90 90 90 [ 	]*cmovns -0x6f6f6f70\(%eax\),%edx
[ 	]*[a-f0-9]+:	0f 4a 90 90 90 90 90 [ 	]*cmovp  -0x6f6f6f70\(%eax\),%edx
[ 	]*[a-f0-9]+:	0f 4b 90 90 90 90 90 [ 	]*cmovnp -0x6f6f6f70\(%eax\),%edx
[ 	]*[a-f0-9]+:	0f 4c 90 90 90 90 90 [ 	]*cmovl  -0x6f6f6f70\(%eax\),%edx
[ 	]*[a-f0-9]+:	0f 4d 90 90 90 90 90 [ 	]*cmovge -0x6f6f6f70\(%eax\),%edx
[ 	]*[a-f0-9]+:	0f 4e 90 90 90 90 90 [ 	]*cmovle -0x6f6f6f70\(%eax\),%edx
[ 	]*[a-f0-9]+:	0f 4f 90 90 90 90 90 [ 	]*cmovg  -0x6f6f6f70\(%eax\),%edx
[ 	]*[a-f0-9]+:	0f 60 90 90 90 90 90 [ 	]*punpcklbw -0x6f6f6f70\(%eax\),%mm2
[ 	]*[a-f0-9]+:	0f 61 90 90 90 90 90 [ 	]*punpcklwd -0x6f6f6f70\(%eax\),%mm2
[ 	]*[a-f0-9]+:	0f 62 90 90 90 90 90 [ 	]*punpckldq -0x6f6f6f70\(%eax\),%mm2
[ 	]*[a-f0-9]+:	0f 63 90 90 90 90 90 [ 	]*packsswb -0x6f6f6f70\(%eax\),%mm2
[ 	]*[a-f0-9]+:	0f 64 90 90 90 90 90 [ 	]*pcmpgtb -0x6f6f6f70\(%eax\),%mm2
[ 	]*[a-f0-9]+:	0f 65 90 90 90 90 90 [ 	]*pcmpgtw -0x6f6f6f70\(%eax\),%mm2
[ 	]*[a-f0-9]+:	0f 66 90 90 90 90 90 [ 	]*pcmpgtd -0x6f6f6f70\(%eax\),%mm2
[ 	]*[a-f0-9]+:	0f 67 90 90 90 90 90 [ 	]*packuswb -0x6f6f6f70\(%eax\),%mm2
[ 	]*[a-f0-9]+:	0f 68 90 90 90 90 90 [ 	]*punpckhbw -0x6f6f6f70\(%eax\),%mm2
[ 	]*[a-f0-9]+:	0f 69 90 90 90 90 90 [ 	]*punpckhwd -0x6f6f6f70\(%eax\),%mm2
[ 	]*[a-f0-9]+:	0f 6a 90 90 90 90 90 [ 	]*punpckhdq -0x6f6f6f70\(%eax\),%mm2
[ 	]*[a-f0-9]+:	0f 6b 90 90 90 90 90 [ 	]*packssdw -0x6f6f6f70\(%eax\),%mm2
[ 	]*[a-f0-9]+:	0f 6e 90 90 90 90 90 [ 	]*movd   -0x6f6f6f70\(%eax\),%mm2
[ 	]*[a-f0-9]+:	0f 6f 90 90 90 90 90 [ 	]*movq   -0x6f6f6f70\(%eax\),%mm2
[ 	]*[a-f0-9]+:	0f 71 d0 90 [ 	]*psrlw  \$0x90,%mm0
[ 	]*[a-f0-9]+:	0f 72 d0 90 [ 	]*psrld  \$0x90,%mm0
[ 	]*[a-f0-9]+:	0f 73 d0 90 [ 	]*psrlq  \$0x90,%mm0
[ 	]*[a-f0-9]+:	0f 74 90 90 90 90 90 [ 	]*pcmpeqb -0x6f6f6f70\(%eax\),%mm2
[ 	]*[a-f0-9]+:	0f 75 90 90 90 90 90 [ 	]*pcmpeqw -0x6f6f6f70\(%eax\),%mm2
[ 	]*[a-f0-9]+:	0f 76 90 90 90 90 90 [ 	]*pcmpeqd -0x6f6f6f70\(%eax\),%mm2
[ 	]*[a-f0-9]+:	0f 77 [ 	]*emms
[ 	]*[a-f0-9]+:	0f 7e 90 90 90 90 90 [ 	]*movd   %mm2,-0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	0f 7f 90 90 90 90 90 [ 	]*movq   %mm2,-0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	0f 80 90 90 90 90 [ 	]*jo     (0x)?909094e6.*
[ 	]*[a-f0-9]+:	0f 81 90 90 90 90 [ 	]*jno    (0x)?909094ec.*
[ 	]*[a-f0-9]+:	0f 82 90 90 90 90 [ 	]*jb     (0x)?909094f2.*
[ 	]*[a-f0-9]+:	0f 83 90 90 90 90 [ 	]*jae    (0x)?909094f8.*
[ 	]*[a-f0-9]+:	0f 84 90 90 90 90 [ 	]*je     (0x)?909094fe.*
[ 	]*[a-f0-9]+:	0f 85 90 90 90 90 [ 	]*jne    (0x)?90909504.*
[ 	]*[a-f0-9]+:	0f 86 90 90 90 90 [ 	]*jbe    (0x)?9090950a.*
[ 	]*[a-f0-9]+:	0f 87 90 90 90 90 [ 	]*ja     (0x)?90909510.*
[ 	]*[a-f0-9]+:	0f 88 90 90 90 90 [ 	]*js     (0x)?90909516.*
[ 	]*[a-f0-9]+:	0f 89 90 90 90 90 [ 	]*jns    (0x)?9090951c.*
[ 	]*[a-f0-9]+:	0f 8a 90 90 90 90 [ 	]*jp     (0x)?90909522.*
[ 	]*[a-f0-9]+:	0f 8b 90 90 90 90 [ 	]*jnp    (0x)?90909528.*
[ 	]*[a-f0-9]+:	0f 8c 90 90 90 90 [ 	]*jl     (0x)?9090952e.*
[ 	]*[a-f0-9]+:	0f 8d 90 90 90 90 [ 	]*jge    (0x)?90909534.*
[ 	]*[a-f0-9]+:	0f 8e 90 90 90 90 [ 	]*jle    (0x)?9090953a.*
[ 	]*[a-f0-9]+:	0f 8f 90 90 90 90 [ 	]*jg     (0x)?90909540.*
[ 	]*[a-f0-9]+:	0f 90 80 90 90 90 90 [ 	]*seto   -0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	0f 91 80 90 90 90 90 [ 	]*setno  -0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	0f 92 80 90 90 90 90 [ 	]*setb   -0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	0f 93 80 90 90 90 90 [ 	]*setae  -0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	0f 94 80 90 90 90 90 [ 	]*sete   -0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	0f 95 80 90 90 90 90 [ 	]*setne  -0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	0f 96 80 90 90 90 90 [ 	]*setbe  -0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	0f 97 80 90 90 90 90 [ 	]*seta   -0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	0f 98 80 90 90 90 90 [ 	]*sets   -0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	0f 99 80 90 90 90 90 [ 	]*setns  -0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	0f 9a 80 90 90 90 90 [ 	]*setp   -0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	0f 9b 80 90 90 90 90 [ 	]*setnp  -0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	0f 9c 80 90 90 90 90 [ 	]*setl   -0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	0f 9d 80 90 90 90 90 [ 	]*setge  -0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	0f 9e 80 90 90 90 90 [ 	]*setle  -0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	0f 9f 80 90 90 90 90 [ 	]*setg   -0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	0f a0 [ 	]*push   %fs
[ 	]*[a-f0-9]+:	0f a1 [ 	]*pop    %fs
[ 	]*[a-f0-9]+:	0f a2 [ 	]*cpuid
[ 	]*[a-f0-9]+:	0f a3 90 90 90 90 90 [ 	]*bt     %edx,-0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	0f a4 90 90 90 90 90 90 [ 	]*shld   \$0x90,%edx,-0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	0f a5 90 90 90 90 90 [ 	]*shld   %cl,%edx,-0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	0f a8 [ 	]*push   %gs
[ 	]*[a-f0-9]+:	0f a9 [ 	]*pop    %gs
[ 	]*[a-f0-9]+:	0f aa [ 	]*rsm
[ 	]*[a-f0-9]+:	0f ab 90 90 90 90 90 [ 	]*bts    %edx,-0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	0f ac 90 90 90 90 90 90 [ 	]*shrd   \$0x90,%edx,-0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	0f ad 90 90 90 90 90 [ 	]*shrd   %cl,%edx,-0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	0f af 90 90 90 90 90 [ 	]*imul   -0x6f6f6f70\(%eax\),%edx
[ 	]*[a-f0-9]+:	0f b0 90 90 90 90 90 [ 	]*cmpxchg %dl,-0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	0f b1 90 90 90 90 90 [ 	]*cmpxchg %edx,-0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	0f b2 90 90 90 90 90 [ 	]*lss    -0x6f6f6f70\(%eax\),%edx
[ 	]*[a-f0-9]+:	0f b3 90 90 90 90 90 [ 	]*btr    %edx,-0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	0f b4 90 90 90 90 90 [ 	]*lfs    -0x6f6f6f70\(%eax\),%edx
[ 	]*[a-f0-9]+:	0f b5 90 90 90 90 90 [ 	]*lgs    -0x6f6f6f70\(%eax\),%edx
[ 	]*[a-f0-9]+:	0f b6 90 90 90 90 90 [ 	]*movzbl -0x6f6f6f70\(%eax\),%edx
[ 	]*[a-f0-9]+:	0f b7 90 90 90 90 90 [ 	]*movzwl -0x6f6f6f70\(%eax\),%edx
[ 	]*[a-f0-9]+:	0f 0b [ 	]*ud2
[ 	]*[a-f0-9]+:	0f bb 90 90 90 90 90 [ 	]*btc    %edx,-0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	0f bc 90 90 90 90 90 [ 	]*bsf    -0x6f6f6f70\(%eax\),%edx
[ 	]*[a-f0-9]+:	0f bd 90 90 90 90 90 [ 	]*bsr    -0x6f6f6f70\(%eax\),%edx
[ 	]*[a-f0-9]+:	0f be 90 90 90 90 90 [ 	]*movsbl -0x6f6f6f70\(%eax\),%edx
[ 	]*[a-f0-9]+:	0f bf 90 90 90 90 90 [ 	]*movswl -0x6f6f6f70\(%eax\),%edx
[ 	]*[a-f0-9]+:	0f c0 90 90 90 90 90 [ 	]*xadd   %dl,-0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	0f c1 90 90 90 90 90 [ 	]*xadd   %edx,-0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	0f c8 [ 	]*bswap  %eax
[ 	]*[a-f0-9]+:	0f c9 [ 	]*bswap  %ecx
[ 	]*[a-f0-9]+:	0f ca [ 	]*bswap  %edx
[ 	]*[a-f0-9]+:	0f cb [ 	]*bswap  %ebx
[ 	]*[a-f0-9]+:	0f cc [ 	]*bswap  %esp
[ 	]*[a-f0-9]+:	0f cd [ 	]*bswap  %ebp
[ 	]*[a-f0-9]+:	0f ce [ 	]*bswap  %esi
[ 	]*[a-f0-9]+:	0f cf [ 	]*bswap  %edi
[ 	]*[a-f0-9]+:	0f d1 90 90 90 90 90 [ 	]*psrlw  -0x6f6f6f70\(%eax\),%mm2
[ 	]*[a-f0-9]+:	0f d2 90 90 90 90 90 [ 	]*psrld  -0x6f6f6f70\(%eax\),%mm2
[ 	]*[a-f0-9]+:	0f d3 90 90 90 90 90 [ 	]*psrlq  -0x6f6f6f70\(%eax\),%mm2
[ 	]*[a-f0-9]+:	0f d5 90 90 90 90 90 [ 	]*pmullw -0x6f6f6f70\(%eax\),%mm2
[ 	]*[a-f0-9]+:	0f d8 90 90 90 90 90 [ 	]*psubusb -0x6f6f6f70\(%eax\),%mm2
[ 	]*[a-f0-9]+:	0f d9 90 90 90 90 90 [ 	]*psubusw -0x6f6f6f70\(%eax\),%mm2
[ 	]*[a-f0-9]+:	0f db 90 90 90 90 90 [ 	]*pand   -0x6f6f6f70\(%eax\),%mm2
[ 	]*[a-f0-9]+:	0f dc 90 90 90 90 90 [ 	]*paddusb -0x6f6f6f70\(%eax\),%mm2
[ 	]*[a-f0-9]+:	0f dd 90 90 90 90 90 [ 	]*paddusw -0x6f6f6f70\(%eax\),%mm2
[ 	]*[a-f0-9]+:	0f df 90 90 90 90 90 [ 	]*pandn  -0x6f6f6f70\(%eax\),%mm2
[ 	]*[a-f0-9]+:	0f e1 90 90 90 90 90 [ 	]*psraw  -0x6f6f6f70\(%eax\),%mm2
[ 	]*[a-f0-9]+:	0f e2 90 90 90 90 90 [ 	]*psrad  -0x6f6f6f70\(%eax\),%mm2
[ 	]*[a-f0-9]+:	0f e5 90 90 90 90 90 [ 	]*pmulhw -0x6f6f6f70\(%eax\),%mm2
[ 	]*[a-f0-9]+:	0f e8 90 90 90 90 90 [ 	]*psubsb -0x6f6f6f70\(%eax\),%mm2
[ 	]*[a-f0-9]+:	0f e9 90 90 90 90 90 [ 	]*psubsw -0x6f6f6f70\(%eax\),%mm2
[ 	]*[a-f0-9]+:	0f eb 90 90 90 90 90 [ 	]*por    -0x6f6f6f70\(%eax\),%mm2
[ 	]*[a-f0-9]+:	0f ec 90 90 90 90 90 [ 	]*paddsb -0x6f6f6f70\(%eax\),%mm2
[ 	]*[a-f0-9]+:	0f ed 90 90 90 90 90 [ 	]*paddsw -0x6f6f6f70\(%eax\),%mm2
[ 	]*[a-f0-9]+:	0f ef 90 90 90 90 90 [ 	]*pxor   -0x6f6f6f70\(%eax\),%mm2
[ 	]*[a-f0-9]+:	0f f1 90 90 90 90 90 [ 	]*psllw  -0x6f6f6f70\(%eax\),%mm2
[ 	]*[a-f0-9]+:	0f f2 90 90 90 90 90 [ 	]*pslld  -0x6f6f6f70\(%eax\),%mm2
[ 	]*[a-f0-9]+:	0f f3 90 90 90 90 90 [ 	]*psllq  -0x6f6f6f70\(%eax\),%mm2
[ 	]*[a-f0-9]+:	0f f5 90 90 90 90 90 [ 	]*pmaddwd -0x6f6f6f70\(%eax\),%mm2
[ 	]*[a-f0-9]+:	0f f8 90 90 90 90 90 [ 	]*psubb  -0x6f6f6f70\(%eax\),%mm2
[ 	]*[a-f0-9]+:	0f f9 90 90 90 90 90 [ 	]*psubw  -0x6f6f6f70\(%eax\),%mm2
[ 	]*[a-f0-9]+:	0f fa 90 90 90 90 90 [ 	]*psubd  -0x6f6f6f70\(%eax\),%mm2
[ 	]*[a-f0-9]+:	0f fc 90 90 90 90 90 [ 	]*paddb  -0x6f6f6f70\(%eax\),%mm2
[ 	]*[a-f0-9]+:	0f fd 90 90 90 90 90 [ 	]*paddw  -0x6f6f6f70\(%eax\),%mm2
[ 	]*[a-f0-9]+:	0f fe 90 90 90 90 90 [ 	]*paddd  -0x6f6f6f70\(%eax\),%mm2
[ 	]*[a-f0-9]+:	66 01 90 90 90 90 90 [ 	]*add    %dx,-0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	66 03 90 90 90 90 90 [ 	]*add    -0x6f6f6f70\(%eax\),%dx
[ 	]*[a-f0-9]+:	66 05 90 90 [ 	]*add    \$0x9090,%ax
[ 	]*[a-f0-9]+:	66 06 [ 	]*pushw  %es
[ 	]*[a-f0-9]+:	66 07 [ 	]*popw   %es
[ 	]*[a-f0-9]+:	66 09 90 90 90 90 90 [ 	]*or     %dx,-0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	66 0b 90 90 90 90 90 [ 	]*or     -0x6f6f6f70\(%eax\),%dx
[ 	]*[a-f0-9]+:	66 0d 90 90 [ 	]*or     \$0x9090,%ax
[ 	]*[a-f0-9]+:	66 0e [ 	]*pushw  %cs
[ 	]*[a-f0-9]+:	66 11 90 90 90 90 90 [ 	]*adc    %dx,-0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	66 13 90 90 90 90 90 [ 	]*adc    -0x6f6f6f70\(%eax\),%dx
[ 	]*[a-f0-9]+:	66 15 90 90 [ 	]*adc    \$0x9090,%ax
[ 	]*[a-f0-9]+:	66 16 [ 	]*pushw  %ss
[ 	]*[a-f0-9]+:	66 17 [ 	]*popw   %ss
[ 	]*[a-f0-9]+:	66 19 90 90 90 90 90 [ 	]*sbb    %dx,-0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	66 1b 90 90 90 90 90 [ 	]*sbb    -0x6f6f6f70\(%eax\),%dx
[ 	]*[a-f0-9]+:	66 1d 90 90 [ 	]*sbb    \$0x9090,%ax
[ 	]*[a-f0-9]+:	66 1e [ 	]*pushw  %ds
[ 	]*[a-f0-9]+:	66 1f [ 	]*popw   %ds
[ 	]*[a-f0-9]+:	66 21 90 90 90 90 90 [ 	]*and    %dx,-0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	66 23 90 90 90 90 90 [ 	]*and    -0x6f6f6f70\(%eax\),%dx
[ 	]*[a-f0-9]+:	66 25 90 90 [ 	]*and    \$0x9090,%ax
[ 	]*[a-f0-9]+:	66 29 90 90 90 90 90 [ 	]*sub    %dx,-0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	66 2b 90 90 90 90 90 [ 	]*sub    -0x6f6f6f70\(%eax\),%dx
[ 	]*[a-f0-9]+:	66 2d 90 90 [ 	]*sub    \$0x9090,%ax
[ 	]*[a-f0-9]+:	66 31 90 90 90 90 90 [ 	]*xor    %dx,-0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	66 33 90 90 90 90 90 [ 	]*xor    -0x6f6f6f70\(%eax\),%dx
[ 	]*[a-f0-9]+:	66 35 90 90 [ 	]*xor    \$0x9090,%ax
[ 	]*[a-f0-9]+:	66 39 90 90 90 90 90 [ 	]*cmp    %dx,-0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	66 3b 90 90 90 90 90 [ 	]*cmp    -0x6f6f6f70\(%eax\),%dx
[ 	]*[a-f0-9]+:	66 3d 90 90 [ 	]*cmp    \$0x9090,%ax
[ 	]*[a-f0-9]+:	66 40 [ 	]*inc    %ax
[ 	]*[a-f0-9]+:	66 41 [ 	]*inc    %cx
[ 	]*[a-f0-9]+:	66 42 [ 	]*inc    %dx
[ 	]*[a-f0-9]+:	66 43 [ 	]*inc    %bx
[ 	]*[a-f0-9]+:	66 44 [ 	]*inc    %sp
[ 	]*[a-f0-9]+:	66 45 [ 	]*inc    %bp
[ 	]*[a-f0-9]+:	66 46 [ 	]*inc    %si
[ 	]*[a-f0-9]+:	66 47 [ 	]*inc    %di
[ 	]*[a-f0-9]+:	66 48 [ 	]*dec    %ax
[ 	]*[a-f0-9]+:	66 49 [ 	]*dec    %cx
[ 	]*[a-f0-9]+:	66 4a [ 	]*dec    %dx
[ 	]*[a-f0-9]+:	66 4b [ 	]*dec    %bx
[ 	]*[a-f0-9]+:	66 4c [ 	]*dec    %sp
[ 	]*[a-f0-9]+:	66 4d [ 	]*dec    %bp
[ 	]*[a-f0-9]+:	66 4e [ 	]*dec    %si
[ 	]*[a-f0-9]+:	66 4f [ 	]*dec    %di
[ 	]*[a-f0-9]+:	66 50 [ 	]*push   %ax
[ 	]*[a-f0-9]+:	66 51 [ 	]*push   %cx
[ 	]*[a-f0-9]+:	66 52 [ 	]*push   %dx
[ 	]*[a-f0-9]+:	66 53 [ 	]*push   %bx
[ 	]*[a-f0-9]+:	66 54 [ 	]*push   %sp
[ 	]*[a-f0-9]+:	66 55 [ 	]*push   %bp
[ 	]*[a-f0-9]+:	66 56 [ 	]*push   %si
[ 	]*[a-f0-9]+:	66 57 [ 	]*push   %di
[ 	]*[a-f0-9]+:	66 58 [ 	]*pop    %ax
[ 	]*[a-f0-9]+:	66 59 [ 	]*pop    %cx
[ 	]*[a-f0-9]+:	66 5a [ 	]*pop    %dx
[ 	]*[a-f0-9]+:	66 5b [ 	]*pop    %bx
[ 	]*[a-f0-9]+:	66 5c [ 	]*pop    %sp
[ 	]*[a-f0-9]+:	66 5d [ 	]*pop    %bp
[ 	]*[a-f0-9]+:	66 5e [ 	]*pop    %si
[ 	]*[a-f0-9]+:	66 5f [ 	]*pop    %di
[ 	]*[a-f0-9]+:	66 60 [ 	]*pushaw
[ 	]*[a-f0-9]+:	66 61 [ 	]*popaw
[ 	]*[a-f0-9]+:	66 62 90 90 90 90 90 [ 	]*bound  %dx,-0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	66 68 90 90 [ 	]*pushw  \$0x9090
[ 	]*[a-f0-9]+:	66 69 90 90 90 90 90 90 90 [ 	]*imul   \$0x9090,-0x6f6f6f70\(%eax\),%dx
[ 	]*[a-f0-9]+:	66 6a 90 [ 	]*pushw  \$0xff90
[ 	]*[a-f0-9]+:	66 6b 90 90 90 90 90 90 [ 	]*imul   \$0xff90,-0x6f6f6f70\(%eax\),%dx
[ 	]*[a-f0-9]+:	66 6d [ 	]*insw   \(%dx\),%es:\(%edi\)
[ 	]*[a-f0-9]+:	66 6f [ 	]*outsw  %ds:\(%esi\),\(%dx\)
[ 	]*[a-f0-9]+:	66 81 90 90 90 90 90 90 90 [ 	]*adcw   \$0x9090,-0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	66 83 90 90 90 90 90 90 [ 	]*adcw   \$0xff90,-0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	66 85 90 90 90 90 90 [ 	]*test   %dx,-0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	66 87 90 90 90 90 90 [ 	]*xchg   %dx,-0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	66 89 90 90 90 90 90 [ 	]*mov    %dx,-0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	66 8b 90 90 90 90 90 [ 	]*mov    -0x6f6f6f70\(%eax\),%dx
[ 	]*[a-f0-9]+:	8c 90 90 90 90 90 [ 	]*mov[w ]   %ss,-0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	66 8d 90 90 90 90 90 [ 	]*lea    -0x6f6f6f70\(%eax\),%dx
[ 	]*[a-f0-9]+:	66 8f 80 90 90 90 90 [ 	]*popw   -0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	66 91 [ 	]*xchg   %ax,%cx
[ 	]*[a-f0-9]+:	66 92 [ 	]*xchg   %ax,%dx
[ 	]*[a-f0-9]+:	66 93 [ 	]*xchg   %ax,%bx
[ 	]*[a-f0-9]+:	66 94 [ 	]*xchg   %ax,%sp
[ 	]*[a-f0-9]+:	66 95 [ 	]*xchg   %ax,%bp
[ 	]*[a-f0-9]+:	66 96 [ 	]*xchg   %ax,%si
[ 	]*[a-f0-9]+:	66 97 [ 	]*xchg   %ax,%di
[ 	]*[a-f0-9]+:	66 98 [ 	]*cbtw
[ 	]*[a-f0-9]+:	66 99 [ 	]*cwtd
[ 	]*[a-f0-9]+:	66 9a 90 90 90 90 [ 	]*lcallw \$0x9090,\$0x9090
[ 	]*[a-f0-9]+:	66 9c [ 	]*pushfw
[ 	]*[a-f0-9]+:	66 9d [ 	]*popfw
[ 	]*[a-f0-9]+:	66 a1 90 90 90 90 [ 	]*mov    0x90909090,%ax
[ 	]*[a-f0-9]+:	66 a3 90 90 90 90 [ 	]*mov    %ax,0x90909090
[ 	]*[a-f0-9]+:	66 a5 [ 	]*movsw  %ds:\(%esi\),%es:\(%edi\)
[ 	]*[a-f0-9]+:	66 a7 [ 	]*cmpsw  %es:\(%edi\),%ds:\(%esi\)
[ 	]*[a-f0-9]+:	66 a9 90 90 [ 	]*test   \$0x9090,%ax
[ 	]*[a-f0-9]+:	66 ab [ 	]*stos   %ax,%es:\(%edi\)
[ 	]*[a-f0-9]+:	66 ad [ 	]*lods   %ds:\(%esi\),%ax
[ 	]*[a-f0-9]+:	66 af [ 	]*scas   %es:\(%edi\),%ax
[ 	]*[a-f0-9]+:	66 b8 90 90 [ 	]*mov    \$0x9090,%ax
[ 	]*[a-f0-9]+:	66 b9 90 90 [ 	]*mov    \$0x9090,%cx
[ 	]*[a-f0-9]+:	66 ba 90 90 [ 	]*mov    \$0x9090,%dx
[ 	]*[a-f0-9]+:	66 bb 90 90 [ 	]*mov    \$0x9090,%bx
[ 	]*[a-f0-9]+:	66 bc 90 90 [ 	]*mov    \$0x9090,%sp
[ 	]*[a-f0-9]+:	66 bd 90 90 [ 	]*mov    \$0x9090,%bp
[ 	]*[a-f0-9]+:	66 be 90 90 [ 	]*mov    \$0x9090,%si
[ 	]*[a-f0-9]+:	66 bf 90 90 [ 	]*mov    \$0x9090,%di
[ 	]*[a-f0-9]+:	66 c1 90 90 90 90 90 90 [ 	]*rclw   \$0x90,-0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	66 c2 90 90 [ 	]*retw   \$0x9090
[ 	]*[a-f0-9]+:	66 c3 [ 	]*retw
[ 	]*[a-f0-9]+:	66 c4 90 90 90 90 90 [ 	]*les    -0x6f6f6f70\(%eax\),%dx
[ 	]*[a-f0-9]+:	66 c5 90 90 90 90 90 [ 	]*lds    -0x6f6f6f70\(%eax\),%dx
[ 	]*[a-f0-9]+:	66 c7 80 90 90 90 90 90 90 [ 	]*movw   \$0x9090,-0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	66 c8 90 90 90 [ 	]*enterw \$0x9090,\$0x90
[ 	]*[a-f0-9]+:	66 c9 [ 	]*leavew
[ 	]*[a-f0-9]+:	66 ca 90 90 [ 	]*lretw  \$0x9090
[ 	]*[a-f0-9]+:	66 cb [ 	]*lretw
[ 	]*[a-f0-9]+:	66 ca 90 90 [ 	]*lretw  \$0x9090
[ 	]*[a-f0-9]+:	66 cb [ 	]*lretw
[ 	]*[a-f0-9]+:	66 cf [ 	]*iretw
[ 	]*[a-f0-9]+:	66 d1 90 90 90 90 90 [ 	]*rclw   -0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	66 d3 90 90 90 90 90 [ 	]*rclw   %cl,-0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	66 e5 90 [ 	]*in     \$0x90,%ax
[ 	]*[a-f0-9]+:	66 e7 90 [ 	]*out    %ax,\$0x90
[ 	]*[a-f0-9]+:	66 e8 8f 90 [ 	]*callw  (0x)?9922.*
[ 	]*[a-f0-9]+:	66 ea 90 90 90 90 [ 	]*ljmpw  \$0x9090,\$0x9090
[ 	]*[a-f0-9]+:	66 ed [ 	]*in     \(%dx\),%ax
[ 	]*[a-f0-9]+:	66 ef [ 	]*out    %ax,\(%dx\)
[ 	]*[a-f0-9]+:	66 f7 90 90 90 90 90 [ 	]*notw   -0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	66 ff 90 90 90 90 90 [ 	]*callw  \*-0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	66 0f 02 90 90 90 90 90 [ 	]*lar    -0x6f6f6f70\(%eax\),%dx
[ 	]*[a-f0-9]+:	66 0f 03 90 90 90 90 90 [ 	]*lsl    -0x6f6f6f70\(%eax\),%dx
[ 	]*[a-f0-9]+:	66 0f 40 90 90 90 90 90 [ 	]*cmovo  -0x6f6f6f70\(%eax\),%dx
[ 	]*[a-f0-9]+:	66 0f 41 90 90 90 90 90 [ 	]*cmovno -0x6f6f6f70\(%eax\),%dx
[ 	]*[a-f0-9]+:	66 0f 42 90 90 90 90 90 [ 	]*cmovb  -0x6f6f6f70\(%eax\),%dx
[ 	]*[a-f0-9]+:	66 0f 43 90 90 90 90 90 [ 	]*cmovae -0x6f6f6f70\(%eax\),%dx
[ 	]*[a-f0-9]+:	66 0f 44 90 90 90 90 90 [ 	]*cmove  -0x6f6f6f70\(%eax\),%dx
[ 	]*[a-f0-9]+:	66 0f 45 90 90 90 90 90 [ 	]*cmovne -0x6f6f6f70\(%eax\),%dx
[ 	]*[a-f0-9]+:	66 0f 46 90 90 90 90 90 [ 	]*cmovbe -0x6f6f6f70\(%eax\),%dx
[ 	]*[a-f0-9]+:	66 0f 47 90 90 90 90 90 [ 	]*cmova  -0x6f6f6f70\(%eax\),%dx
[ 	]*[a-f0-9]+:	66 0f 48 90 90 90 90 90 [ 	]*cmovs  -0x6f6f6f70\(%eax\),%dx
[ 	]*[a-f0-9]+:	66 0f 49 90 90 90 90 90 [ 	]*cmovns -0x6f6f6f70\(%eax\),%dx
[ 	]*[a-f0-9]+:	66 0f 4a 90 90 90 90 90 [ 	]*cmovp  -0x6f6f6f70\(%eax\),%dx
[ 	]*[a-f0-9]+:	66 0f 4b 90 90 90 90 90 [ 	]*cmovnp -0x6f6f6f70\(%eax\),%dx
[ 	]*[a-f0-9]+:	66 0f 4c 90 90 90 90 90 [ 	]*cmovl  -0x6f6f6f70\(%eax\),%dx
[ 	]*[a-f0-9]+:	66 0f 4d 90 90 90 90 90 [ 	]*cmovge -0x6f6f6f70\(%eax\),%dx
[ 	]*[a-f0-9]+:	66 0f 4e 90 90 90 90 90 [ 	]*cmovle -0x6f6f6f70\(%eax\),%dx
[ 	]*[a-f0-9]+:	66 0f 4f 90 90 90 90 90 [ 	]*cmovg  -0x6f6f6f70\(%eax\),%dx
[ 	]*[a-f0-9]+:	66 0f a0 [ 	]*pushw  %fs
[ 	]*[a-f0-9]+:	66 0f a1 [ 	]*popw   %fs
[ 	]*[a-f0-9]+:	66 0f a3 90 90 90 90 90 [ 	]*bt     %dx,-0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	66 0f a4 90 90 90 90 90 90 [ 	]*shld   \$0x90,%dx,-0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	66 0f a5 90 90 90 90 90 [ 	]*shld   %cl,%dx,-0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	66 0f a8 [ 	]*pushw  %gs
[ 	]*[a-f0-9]+:	66 0f a9 [ 	]*popw   %gs
[ 	]*[a-f0-9]+:	66 0f ab 90 90 90 90 90 [ 	]*bts    %dx,-0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	66 0f ac 90 90 90 90 90 90 [ 	]*shrd   \$0x90,%dx,-0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	66 0f ad 90 90 90 90 90 [ 	]*shrd   %cl,%dx,-0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	66 0f af 90 90 90 90 90 [ 	]*imul   -0x6f6f6f70\(%eax\),%dx
[ 	]*[a-f0-9]+:	66 0f b1 90 90 90 90 90 [ 	]*cmpxchg %dx,-0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	66 0f b2 90 90 90 90 90 [ 	]*lss    -0x6f6f6f70\(%eax\),%dx
[ 	]*[a-f0-9]+:	66 0f b3 90 90 90 90 90 [ 	]*btr    %dx,-0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	66 0f b4 90 90 90 90 90 [ 	]*lfs    -0x6f6f6f70\(%eax\),%dx
[ 	]*[a-f0-9]+:	66 0f b5 90 90 90 90 90 [ 	]*lgs    -0x6f6f6f70\(%eax\),%dx
[ 	]*[a-f0-9]+:	66 0f b6 90 90 90 90 90 [ 	]*movzbw -0x6f6f6f70\(%eax\),%dx
[ 	]*[a-f0-9]+:	66 0f bb 90 90 90 90 90 [ 	]*btc    %dx,-0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	66 0f bc 90 90 90 90 90 [ 	]*bsf    -0x6f6f6f70\(%eax\),%dx
[ 	]*[a-f0-9]+:	66 0f bd 90 90 90 90 90 [ 	]*bsr    -0x6f6f6f70\(%eax\),%dx
[ 	]*[a-f0-9]+:	66 0f be 90 90 90 90 90 [ 	]*movsbw -0x6f6f6f70\(%eax\),%dx
[ 	]*[a-f0-9]+:	66 0f c1 90 90 90 90 90 [ 	]*xadd   %dx,-0x6f6f6f70\(%eax\)

[a-f0-9]+ <gs_foo>:
[ 	]*[a-f0-9]+:	c3 [ 	]*ret

[a-f0-9]+ <short_foo>:
[ 	]*[a-f0-9]+:	c3 [ 	]*ret

[a-f0-9]+ <bar>:
[ 	]*[a-f0-9]+:	e8 f9 ff ff ff [ 	]*call   9d9 <gs_foo>
[ 	]*[a-f0-9]+:	e8 f5 ff ff ff [ 	]*call   9da <short_foo>
[ 	]*[a-f0-9]+:	dd 1c d0 [ 	]*fstpl  \(%eax,%edx,8\)
[ 	]*[a-f0-9]+:	b9 00 00 00 00 [ 	]*mov    \$0x0,%ecx
[ 	]*[a-f0-9]+:	88 04 16 [ 	]*mov    %al,\(%esi,%edx,1\)
[ 	]*[a-f0-9]+:	88 04 32 [ 	]*mov    %al,\(%edx,%esi,1\)
[ 	]*[a-f0-9]+:	88 04 56 [ 	]*mov    %al,\(%esi,%edx,2\)
[ 	]*[a-f0-9]+:	88 04 56 [ 	]*mov    %al,\(%esi,%edx,2\)
[ 	]*[a-f0-9]+:	eb 0c [ 	]*jmp    a07 <rot5>
[ 	]*[a-f0-9]+:	6c [ 	]*insb   \(%dx\),%es:\(%edi\)
[ 	]*[a-f0-9]+:	66 0f c1 90 90 90 90 90 [ 	]*xadd   %dx,-0x6f6f6f70\(%eax\)
[ 	]*[a-f0-9]+:	83 e0 f8 [ 	]*and    \$0xfffffff8,%eax

[a-f0-9]+ <rot5>:
[ 	]*[a-f0-9]+:	8b 44 ce 04 [ 	]*mov    0x4\(%esi,%ecx,8\),%eax
[ 	]*[a-f0-9]+:	6c [ 	]*insb   \(%dx\),%es:\(%edi\)
[ 	]*[a-f0-9]+:	0c 90 [ 	]*or     \$0x90,%al
[ 	]*[a-f0-9]+:	0d 90 90 90 90 [ 	]*or     \$0x90909090,%eax
[ 	]*[a-f0-9]+:	0e [ 	]*push   %cs
[ 	]*[a-f0-9]+:	8b 04 5d 00 00 00 00 [ 	]*mov    0x0\(,%ebx,2\),%eax
[ 	]*[a-f0-9]+:	10 14 85 90 90 90 90 [ 	]*adc    %dl,-0x6f6f6f70\(,%eax,4\)
[ 	]*[a-f0-9]+:	2f [ 	]*das
[ 	]*[a-f0-9]+:	ea 90 90 90 90 90 90 [ 	]*ljmp   \$0x9090,\$0x90909090
[ 	]*[a-f0-9]+:	66 a5 [ 	]*movsw  %ds:\(%esi\),%es:\(%edi\)
[ 	]*[a-f0-9]+:	70 90 [ 	]*jo     9be <foo\+0x9be>
[ 	]*[a-f0-9]+:	75 fe [ 	]*jne    a2e <rot5\+0x27>
[ 	]*[a-f0-9]+:	0f 6f 35 28 00 00 00 [ 	]*movq   0x28,%mm6
[ 	]*[a-f0-9]+:	03 3c c3 [ 	]*add    \(%ebx,%eax,8\),%edi
[ 	]*[a-f0-9]+:	0f 6e 44 c3 04 [ 	]*movd   0x4\(%ebx,%eax,8\),%mm0
[ 	]*[a-f0-9]+:	03 bc cb 00 80 00 00 [ 	]*add    0x8000\(%ebx,%ecx,8\),%edi
[ 	]*[a-f0-9]+:	0f 6e 8c cb 04 80 00 00 [ 	]*movd   0x8004\(%ebx,%ecx,8\),%mm1
[ 	]*[a-f0-9]+:	0f 6e 94 c3 04 00 01 00 [ 	]*movd   0x10004\(%ebx,%eax,8\),%mm2
[ 	]*[a-f0-9]+:	03 bc c3 00 00 01 00 [ 	]*add    0x10000\(%ebx,%eax,8\),%edi
[ 	]*[a-f0-9]+:	66 8b 04 43 [ 	]*mov    \(%ebx,%eax,2\),%ax
[ 	]*[a-f0-9]+:	66 8b 8c 4b 00 20 00 00 [ 	]*mov    0x2000\(%ebx,%ecx,2\),%cx
[ 	]*[a-f0-9]+:	66 8b 84 43 00 40 00 00 [ 	]*mov    0x4000\(%ebx,%eax,2\),%ax
[ 	]*[a-f0-9]+:	ff e0 [ 	]*jmp    \*%eax
[ 	]*[a-f0-9]+:	ff 20 [ 	]*jmp    \*\(%eax\)
[ 	]*[a-f0-9]+:	ff 25 db 09 00 00 [ 	]*jmp    \*0x9db
[ 	]*[a-f0-9]+:	e9 5b ff ff ff [ 	]*jmp    9db <bar>
[ 	]*[a-f0-9]+:	b8 12 00 00 00 [ 	]*mov    \$0x12,%eax
[ 	]*[a-f0-9]+:	25 ff ff fb ff [ 	]*and    \$0xfffbffff,%eax
[ 	]*[a-f0-9]+:	25 ff ff fb ff [ 	]*and    \$0xfffbffff,%eax
[ 	]*[a-f0-9]+:	b0 11 [ 	]*mov    \$0x11,%al
[ 	]*[a-f0-9]+:	b0 11 [ 	]*mov    \$0x11,%al
[ 	]*[a-f0-9]+:	b3 47 [ 	]*mov    \$0x47,%bl
[ 	]*[a-f0-9]+:	b3 47 [ 	]*mov    \$0x47,%bl
[ 	]*[a-f0-9]+:	0f ad d0 [ 	]*shrd   %cl,%edx,%eax
[ 	]*[a-f0-9]+:	0f a5 d0 [ 	]*shld   %cl,%edx,%eax
[ 	]*[a-f0-9]+:	de c1                	faddp  %st,%st\(1\)
[ 	]*[a-f0-9]+:	d8 c3                	fadd   %st\(3\),%st
[ 	]*[a-f0-9]+:	d8 c3                	fadd   %st\(3\),%st
[ 	]*[a-f0-9]+:	dc c3                	fadd   %st,%st\(3\)
[ 	]*[a-f0-9]+:	d8 03                	fadds  \(%ebx\)
[ 	]*[a-f0-9]+:	dc 03                	faddl  \(%ebx\)
[ 	]*[a-f0-9]+:	de c1                	faddp  %st,%st\(1\)
[ 	]*[a-f0-9]+:	de c3                	faddp  %st,%st\(3\)
[ 	]*[a-f0-9]+:	de c3                	faddp  %st,%st\(3\)
[ 	]*[a-f0-9]+:	de f9                	fdivrp %st,%st\(1\)
[ 	]*[a-f0-9]+:	d8 f3                	fdiv   %st\(3\),%st
[ 	]*[a-f0-9]+:	d8 f3                	fdiv   %st\(3\),%st
[ 	]*[a-f0-9]+:	dc fb                	fdivr  %st,%st\(3\)
[ 	]*[a-f0-9]+:	d8 33                	fdivs  \(%ebx\)
[ 	]*[a-f0-9]+:	dc 33                	fdivl  \(%ebx\)
[ 	]*[a-f0-9]+:	de f9                	fdivrp %st,%st\(1\)
[ 	]*[a-f0-9]+:	de fb                	fdivrp %st,%st\(3\)
[ 	]*[a-f0-9]+:	de fb                	fdivrp %st,%st\(3\)
[ 	]*[a-f0-9]+:	d8 f3                	fdiv   %st\(3\),%st
[ 	]*[a-f0-9]+:	de f1                	fdivp  %st,%st\(1\)
[ 	]*[a-f0-9]+:	d8 fb                	fdivr  %st\(3\),%st
[ 	]*[a-f0-9]+:	d8 fb                	fdivr  %st\(3\),%st
[ 	]*[a-f0-9]+:	dc f3                	fdiv   %st,%st\(3\)
[ 	]*[a-f0-9]+:	d8 3b                	fdivrs \(%ebx\)
[ 	]*[a-f0-9]+:	dc 3b                	fdivrl \(%ebx\)
[ 	]*[a-f0-9]+:	de f1                	fdivp  %st,%st\(1\)
[ 	]*[a-f0-9]+:	de f3                	fdivp  %st,%st\(3\)
[ 	]*[a-f0-9]+:	de f3                	fdivp  %st,%st\(3\)
[ 	]*[a-f0-9]+:	d8 fb                	fdivr  %st\(3\),%st
[ 	]*[a-f0-9]+:	de c9                	fmulp  %st,%st\(1\)
[ 	]*[a-f0-9]+:	d8 cb                	fmul   %st\(3\),%st
[ 	]*[a-f0-9]+:	d8 cb                	fmul   %st\(3\),%st
[ 	]*[a-f0-9]+:	dc cb                	fmul   %st,%st\(3\)
[ 	]*[a-f0-9]+:	d8 0b                	fmuls  \(%ebx\)
[ 	]*[a-f0-9]+:	dc 0b                	fmull  \(%ebx\)
[ 	]*[a-f0-9]+:	de c9                	fmulp  %st,%st\(1\)
[ 	]*[a-f0-9]+:	de cb                	fmulp  %st,%st\(3\)
[ 	]*[a-f0-9]+:	de cb                	fmulp  %st,%st\(3\)
[ 	]*[a-f0-9]+:	de e9                	fsubrp %st,%st\(1\)
[ 	]*[a-f0-9]+:	de e1                	fsubp  %st,%st\(1\)
[ 	]*[a-f0-9]+:	d8 e3                	fsub   %st\(3\),%st
[ 	]*[a-f0-9]+:	d8 e3                	fsub   %st\(3\),%st
[ 	]*[a-f0-9]+:	dc eb                	fsubr  %st,%st\(3\)
[ 	]*[a-f0-9]+:	d8 23                	fsubs  \(%ebx\)
[ 	]*[a-f0-9]+:	dc 23                	fsubl  \(%ebx\)
[ 	]*[a-f0-9]+:	de e9                	fsubrp %st,%st\(1\)
[ 	]*[a-f0-9]+:	de eb                	fsubrp %st,%st\(3\)
[ 	]*[a-f0-9]+:	d8 e3                	fsub   %st\(3\),%st
[ 	]*[a-f0-9]+:	de eb                	fsubrp %st,%st\(3\)
[ 	]*[a-f0-9]+:	d8 eb                	fsubr  %st\(3\),%st
[ 	]*[a-f0-9]+:	d8 eb                	fsubr  %st\(3\),%st
[ 	]*[a-f0-9]+:	dc e3                	fsub   %st,%st\(3\)
[ 	]*[a-f0-9]+:	d8 2b                	fsubrs \(%ebx\)
[ 	]*[a-f0-9]+:	dc 2b                	fsubrl \(%ebx\)
[ 	]*[a-f0-9]+:	de e1                	fsubp  %st,%st\(1\)
[ 	]*[a-f0-9]+:	de e3                	fsubp  %st,%st\(3\)
[ 	]*[a-f0-9]+:	de e3                	fsubp  %st,%st\(3\)
[ 	]*[a-f0-9]+:	d8 eb                	fsubr  %st\(3\),%st
[ 	]*[a-f0-9]+:	de 3b                	fidivrs \(%ebx\)
[ 	]*[a-f0-9]+:	da 3b                	fidivrl \(%ebx\)
[ 	]*[a-f0-9]+:	0f 4a 90 90 90 90 90 	cmovp  -0x6f6f6f70\(%eax\),%edx
[ 	]*[a-f0-9]+:	0f 4b 90 90 90 90 90 	cmovnp -0x6f6f6f70\(%eax\),%edx
[ 	]*[a-f0-9]+:	66 0f 4a 90 90 90 90 90 	cmovp  -0x6f6f6f70\(%eax\),%dx
[ 	]*[a-f0-9]+:	66 0f 4b 90 90 90 90 90 	cmovnp -0x6f6f6f70\(%eax\),%dx
[ 	]*[a-f0-9]+:	0f 02 c0             	lar    %eax,%eax
[ 	]*[a-f0-9]+:	66 0f 02 c0          	lar    %ax,%ax
[ 	]*[a-f0-9]+:	0f 02 00             	lar    \(%eax\),%eax
[ 	]*[a-f0-9]+:	66 0f 02 00          	lar    \(%eax\),%ax
[ 	]*[a-f0-9]+:	0f 03 c0             	lsl    %eax,%eax
[ 	]*[a-f0-9]+:	66 0f 03 c0          	lsl    %ax,%ax
[ 	]*[a-f0-9]+:	0f 03 00             	lsl    \(%eax\),%eax
[ 	]*[a-f0-9]+:	66 0f 03 00          	lsl    \(%eax\),%ax
[ 	]*[a-f0-9]+:	8b 04 04             	mov    \(%esp,%eax(,1)?\),%eax
[ 	]*[a-f0-9]+:	8b 04 20             	mov    \(%eax(,%eiz)?(,1)?\),%eax
[ 	]*[a-f0-9]+:	c4 e2 69 92 04 08    	vgatherdps %xmm2,\(%eax,%xmm1(,1)?\),%xmm0
[ 	]*[a-f0-9]+:	24 2f                	and    \$0x2f,%al
[ 	]*[a-f0-9]+:	0f                   	\.byte 0xf
[a-f0-9]+ <barn>:
[ 	]*[a-f0-9]+:	0f ba e2 03          	bt     \$0x3,%edx
#pass
