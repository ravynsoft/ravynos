 .intel_syntax noprefix
 .code16
 .text

 movsx	eax,word ptr ds:[0]
 movsx	eax,byte ptr ds:[0]
 movsx	ax,byte ptr ds:[0]
 movzx	eax,word ptr ds:[0]
 movzx	eax,byte ptr ds:[0]
 movzx	ax,byte ptr ds:[0]

 lea	ax, [si+bx]
 lea	ax, [si+bp]
 lea	ax, [di+bx]
 lea	ax, [di+bp]
 lea	ax, [si][bx]
 lea	ax, [si][bp]
 lea	ax, [di][bx]
 lea	ax, [di][bp]

 notw	[ebx]
 notd	[bx]

 nopw	[ebx]
 nopd	[bx]

 addw	[ebx], 5
 addd	[bx], 5

 movw	[ebx], 5
 movd	[bx], 5
