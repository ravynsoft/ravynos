	.text
	.code16
	addr32 mov	0x600898,%al
	addr32 mov	0x600898,%ax
	addr32 mov	0x600898,%eax
	addr32 mov	%al,0x600898
	addr32 mov	%ax,0x600898
	addr32 mov	%eax,0x600898
	addr32 movl	$0x1,(%esp)
	addr32 mov	0x89abcdef,%eax
	addr32 mov	0x89abcdef,%ebx
	addr32 mov	$0x89abcdef,%eax
	addr32 mov	$0x89abcdef,%ebx
	addr32 mov	%eax,0x89abcdef
	addr32 mov	%ebx,0x89abcdef
