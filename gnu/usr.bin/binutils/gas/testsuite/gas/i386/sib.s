#Test the special case of the index bits, 0x4, in SIB.

	.text
	.allow_index_reg
foo:
	mov	-30,%ebx
	mov	-30(,%eiz),%ebx
	mov	-30(,%eiz,1),%eax
	mov	-30(,%eiz,2),%eax
	mov	-30(,%eiz,4),%eax
	mov	-30(,%eiz,8),%eax
	mov	30,%eax
	mov	30(,%eiz),%eax
	mov	30(,%eiz,1),%eax
	mov	30(,%eiz,2),%eax
	mov	30(,%eiz,4),%eax
	mov	30(,%eiz,8),%eax
	mov	(%ebx),%eax
	mov	(%ebx,%eiz),%eax
	mov	(%ebx,%eiz,1),%eax
	mov	(%ebx,%eiz,2),%eax
	mov	(%ebx,%eiz,4),%eax
	mov	(%ebx,%eiz,8),%eax
	mov	(%esp),%eax
	mov	(%esp,%eiz,1),%eax
	mov	(%esp,%eiz,2),%eax
	mov	(%esp,%eiz,4),%eax
	mov	(%esp,%eiz,8),%eax
	mov	(%eax, %eax, (1 << 0)), %eax
	mov	(%eax, %eax, (1 << 1)), %eax
	mov	(%eax, %eax, (1 << 2)), %eax
	mov	(%eax, %eax, (1 << 3)), %eax
	.equ "scale(1)", 1
	mov	(%eax, %ecx, "scale(1)"), %edx
	.equiv "scale[2]", 2
	mov	(%eax, %ecx, "scale[2]"), %edx
	.eqv "scale{4}", 4
	mov	(%eax, %ecx, "scale{4}"), %edx
	.set "scale<8>", 8
	mov	(%eax, %ecx, "scale<8>"), %edx
	.intel_syntax noprefix
        mov    eax,DWORD PTR [eiz*1-30]
        mov    eax,DWORD PTR [eiz*2-30]
        mov    eax,DWORD PTR [eiz*4-30]
        mov    eax,DWORD PTR [eiz*8-30]
        mov    eax,DWORD PTR [eiz*1+30]
        mov    eax,DWORD PTR [eiz*2+30]
        mov    eax,DWORD PTR [eiz*4+30]
        mov    eax,DWORD PTR [eiz*8+30]
        mov    eax,DWORD PTR [ebx+eiz]
        mov    eax,DWORD PTR [ebx+eiz*1]
        mov    eax,DWORD PTR [ebx+eiz*2]
        mov    eax,DWORD PTR [ebx+eiz*4]
        mov    eax,DWORD PTR [ebx+eiz*8]
        mov    eax,DWORD PTR [esp]
        mov    eax,DWORD PTR [esp+eiz]
        mov    eax,DWORD PTR [esp+eiz*1]
        mov    eax,DWORD PTR [esp+eiz*2]
        mov    eax,DWORD PTR [esp+eiz*4]
        mov    eax,DWORD PTR [esp+eiz*8]
	.p2align 4
