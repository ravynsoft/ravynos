 .macro bad args:vararg
  .ifdef _bad_
	\args
  .endif
 .endm

 .macro ill args:vararg
  # This is used to mark entries that aren't handled consistently,
  # and thus shouldn't currently be checked for.
  #	\args
 .endm

 .text
_start:
	add	$xtrn, %rax
	mov	$xtrn, %eax
	mov	$xtrn, %ax
	mov	$xtrn, %al
	mov	xtrn(%rbx), %eax
	mov	xtrn(%ebx), %eax

	add	$(xtrn - .), %rax
ill	mov	$(xtrn - .), %eax
	mov	$(xtrn - .), %ax
	mov	$(xtrn - .), %al
	mov	xtrn(%rip), %eax
	mov	xtrn(%eip), %eax
	call	xtrn
	jrcxz	xtrn

	add	$xtrn@got, %rax
	mov	$xtrn@got, %eax
bad	mov	$xtrn@got, %ax
bad	mov	$xtrn@got, %al
	mov	xtrn@got(%rbx), %eax
	mov	xtrn@got(%ebx), %eax
bad	call	xtrn@got

bad	add	$xtrn@gotoff, %rax
bad	mov	$xtrn@gotoff, %eax
bad	mov	$xtrn@gotoff, %ax
bad	mov	$xtrn@gotoff, %al
bad	mov	xtrn@gotoff(%rbx), %eax
bad	mov	xtrn@gotoff(%ebx), %eax
bad	call	xtrn@gotoff

bad	movabs	$xtrn@gotpcrel, %rax
	add	$xtrn@gotpcrel, %rax
	mov	$xtrn@gotpcrel, %eax
bad	mov	$xtrn@gotpcrel, %ax
bad	mov	$xtrn@gotpcrel, %al
	mov	xtrn@gotpcrel(%rbx), %eax
	mov	xtrn@gotpcrel(%ebx), %eax
	call	xtrn@gotpcrel

ill	movabs	$_GLOBAL_OFFSET_TABLE_, %rax
	add	$_GLOBAL_OFFSET_TABLE_, %rax
ill	add	$_GLOBAL_OFFSET_TABLE_, %eax
ill	add	$_GLOBAL_OFFSET_TABLE_, %ax
ill	add	$_GLOBAL_OFFSET_TABLE_, %al
	lea	_GLOBAL_OFFSET_TABLE_(%rip), %rax
	lea	_GLOBAL_OFFSET_TABLE_(%eip), %rax
ill	movabs	$(_GLOBAL_OFFSET_TABLE_ - .), %rax
	add	$(_GLOBAL_OFFSET_TABLE_ - .), %rax
ill	add	$(_GLOBAL_OFFSET_TABLE_ - .), %eax
ill	add	$(_GLOBAL_OFFSET_TABLE_ - .), %ax
ill	add	$(_GLOBAL_OFFSET_TABLE_ - .), %al

bad	movabs	$xtrn@plt, %rax
	add	$xtrn@plt, %rax
	mov	$xtrn@plt, %eax
bad	mov	$xtrn@plt, %ax
bad	mov	$xtrn@plt, %al
	mov	xtrn@plt(%rbx), %eax
	mov	xtrn@plt(%ebx), %eax
	call	xtrn@plt
bad	jrcxz	xtrn@plt

bad	movabs	$xtrn@tlsgd, %rax
	add	$xtrn@tlsgd, %rax
	mov	$xtrn@tlsgd, %eax
bad	mov	$xtrn@tlsgd, %ax
bad	mov	$xtrn@tlsgd, %al
	mov	xtrn@tlsgd(%rbx), %eax
	mov	xtrn@tlsgd(%ebx), %eax
	call	xtrn@tlsgd

bad	movabs	$xtrn@gottpoff, %rax
	add	$xtrn@gottpoff, %rax
	mov	$xtrn@gottpoff, %eax
bad	mov	$xtrn@gottpoff, %ax
bad	mov	$xtrn@gottpoff, %al
	mov	xtrn@gottpoff(%rbx), %eax
	mov	xtrn@gottpoff(%ebx), %eax
	call	xtrn@gottpoff

bad	movabs	$xtrn@tlsld, %rax
	add	$xtrn@tlsld, %rax
	mov	$xtrn@tlsld, %eax
bad	mov	$xtrn@tlsld, %ax
bad	mov	$xtrn@tlsld, %al
	mov	xtrn@tlsld(%rbx), %eax
	mov	xtrn@tlsld(%ebx), %eax
	call	xtrn@tlsld

	add	$xtrn@dtpoff, %rax
	mov	$xtrn@dtpoff, %eax
bad	mov	$xtrn@dtpoff, %ax
bad	mov	$xtrn@dtpoff, %al
	mov	xtrn@dtpoff(%rbx), %eax
	mov	xtrn@dtpoff(%ebx), %eax
bad	call	xtrn@dtpoff

	add	$xtrn@tpoff, %rax
	mov	$xtrn@tpoff, %eax
bad	mov	$xtrn@tpoff, %ax
bad	mov	$xtrn@tpoff, %al
	mov	xtrn@tpoff(%rbx), %eax
	mov	xtrn@tpoff(%ebx), %eax
bad	call	xtrn@tpoff

 .data
	.long	xtrn
	.long	xtrn - .
	.long	xtrn@got
bad	.long	xtrn@gotoff
	.long	xtrn@gotpcrel
	.long	_GLOBAL_OFFSET_TABLE_
	.long	_GLOBAL_OFFSET_TABLE_ - .
	.long	xtrn@plt
	.long	xtrn@tlsgd
	.long	xtrn@gottpoff
	.long	xtrn@tlsld
	.long	xtrn@dtpoff
	.long	xtrn@tpoff
	
	.slong	xtrn
	.slong	xtrn - .
	.slong	xtrn@got
bad	.slong	xtrn@gotoff
	.slong	xtrn@gotpcrel
	.slong	_GLOBAL_OFFSET_TABLE_
	.slong	_GLOBAL_OFFSET_TABLE_ - .
	.slong	xtrn@plt
	.slong	xtrn@tlsgd
	.slong	xtrn@gottpoff
	.slong	xtrn@tlsld
	.slong	xtrn@dtpoff
	.slong	xtrn@tpoff
	
	.word	xtrn
	.word	xtrn - .
bad	.word	xtrn@got
bad	.word	xtrn@gotoff
bad	.word	xtrn@gotpcrel
ill	.word	_GLOBAL_OFFSET_TABLE_
ill	.word	_GLOBAL_OFFSET_TABLE_ - .
bad	.word	xtrn@plt
bad	.word	xtrn@tlsgd
bad	.word	xtrn@gottpoff
bad	.word	xtrn@tlsld
bad	.word	xtrn@dtpoff
bad	.word	xtrn@tpoff

	.byte	xtrn
	.byte	xtrn - .
bad	.byte	xtrn@got
bad	.byte	xtrn@gotoff
bad	.byte	xtrn@gotpcrel
ill	.byte	_GLOBAL_OFFSET_TABLE_
ill	.byte	_GLOBAL_OFFSET_TABLE_ - .
bad	.byte	xtrn@plt
bad	.byte	xtrn@tlsgd
bad	.byte	xtrn@gottpoff
bad	.byte	xtrn@tlsld
bad	.byte	xtrn@dtpoff
bad	.byte	xtrn@tpoff

	.text
	mov	xtrn@tpoff (%rbx), %eax
	movabsq	$xtrn + 0x7fffffff, %rbx
	movabsq	$xtrn - 0x80000000, %rbp

	.data
	.quad	xtrn
	.quad	xtrn + 0x7fffffff
	.quad	xtrn - 0x80000000
	.long	xtrn@got - 4
	.long	xtrn@got + 4
bad	.long	xtrn@plt - .

	.text
bad	add	$x+0x123456789, %rax
bad	add	$x+0x123456789, %eax
bad	add	x+0x123456789, %eax
bad	add	x+0x123456789(%eax), %eax
bad	enter	$x+0x123456789, $x+0x123456789
bad	enter	$x+0x12345, $x+0x123

	.data
bad	.long x+0x123456789
bad	.word x+0x123456789
bad	.word x+0x12345
bad	.byte x+0x123456789
bad	.byte x+0x123
