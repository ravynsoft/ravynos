#Test the special case of the index bits, 0x4, in SIB.

	.text
	.allow_index_reg
foo:
	mov	-30,%ebx
	mov	-30(,%riz),%ebx
	mov	-30(,%riz,1),%eax
	mov	-30(,%riz,2),%eax
	mov	-30(,%riz,4),%eax
	mov	-30(,%riz,8),%eax
	mov	30,%eax
	mov	30(,%riz),%eax
	mov	30(,%riz,1),%eax
	mov	30(,%riz,2),%eax
	mov	30(,%riz,4),%eax
	mov	30(,%riz,8),%eax
	mov	(%rbx),%eax
	mov	(%rbx,%riz),%eax
	mov	(%rbx,%riz,1),%eax
	mov	(%rbx,%riz,2),%eax
	mov	(%rbx,%riz,4),%eax
	mov	(%rbx,%riz,8),%eax
	mov	(%rsp),%eax
	mov	(%rsp,%riz),%eax
	mov	(%rsp,%riz,1),%eax
	mov	(%rsp,%riz,2),%eax
	mov	(%rsp,%riz,4),%eax
	mov	(%rsp,%riz,8),%eax
	mov	(%r12),%eax
	mov	(%r12,%riz),%eax
	mov	(%r12,%riz,1),%eax
	mov	(%r12,%riz,2),%eax
	mov	(%r12,%riz,4),%eax
	mov	(%r12,%riz,8),%eax
	.intel_syntax noprefix
        mov    eax,DWORD PTR [riz*1-30]
        mov    eax,DWORD PTR [riz*2-30]
        mov    eax,DWORD PTR [riz*4-30]
        mov    eax,DWORD PTR [riz*8-30]
        mov    eax,DWORD PTR [riz*1+30]
        mov    eax,DWORD PTR [riz*2+30]
        mov    eax,DWORD PTR [riz*4+30]
        mov    eax,DWORD PTR [riz*8+30]
        mov    eax,DWORD PTR [rbx+riz]
        mov    eax,DWORD PTR [rbx+riz*1]
        mov    eax,DWORD PTR [rbx+riz*2]
        mov    eax,DWORD PTR [rbx+riz*4]
        mov    eax,DWORD PTR [rbx+riz*8]
        mov    eax,DWORD PTR [rsp]
        mov    eax,DWORD PTR [rsp+riz]
        mov    eax,DWORD PTR [rsp+riz*1]
        mov    eax,DWORD PTR [rsp+riz*2]
        mov    eax,DWORD PTR [rsp+riz*4]
        mov    eax,DWORD PTR [rsp+riz*8]
        mov    eax,DWORD PTR [r12]
        mov    eax,DWORD PTR [r12+riz]
        mov    eax,DWORD PTR [r12+riz*1]
        mov    eax,DWORD PTR [r12+riz*2]
        mov    eax,DWORD PTR [r12+riz*4]
        mov    eax,DWORD PTR [r12+riz*8]
