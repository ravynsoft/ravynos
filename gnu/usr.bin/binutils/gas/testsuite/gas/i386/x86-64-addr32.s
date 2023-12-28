	.text
	.allow_index_reg
	lea		symbol(%eax), %rax
	lea		symbol(%r8d), %rax
	lea		symbol(%eip), %rax
	addr32 lea	symbol, %rax
	addr32 mov	0x600898,%al
	addr32 mov	0x600898,%ax
	addr32 mov	0x600898,%eax
	addr32 mov	0x600898,%rax
	addr32 mov	0x800898,%rax
	addr32 mov	0x800898,%rbx
	addr32 mov	0x89abcdef,%rax
	addr32 mov	0x89abcdef,%rbx
	addr32 mov	$0x89abcdef,%rax
	addr32 mov	$0x89abcdef,%rbx
	addr32 mov	%al,0x600898
	addr32 mov	%ax,0x600898
	addr32 mov	%eax,0x600898
	addr32 mov	%rax,0x600898
	addr32 mov	%rax,0x800898
	addr32 mov	%rbx,0x800898
	addr32 mov	%rax,0x89abcdef
	addr32 mov	%rbx,0x89abcdef
	mov		%eax, -0xccddef(,%eiz,)
	mov		%eax, -0xccddef(,%eiz,2)
