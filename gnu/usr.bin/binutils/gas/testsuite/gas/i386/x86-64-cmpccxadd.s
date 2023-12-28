# Check 64bit CMPccXADD instructions

	.allow_index_reg
	.text
_start:
	cmpbexadd	%eax, %ecx, 0x10000000(%rbp, %r14, 8)	 #CMPCCXADD
	cmpbexadd	%ebx, %ecx, (%r9)	 #CMPCCXADD
	cmpbexadd	%eax, %ecx, 508(%rcx)	 #CMPCCXADD Disp32(fc010000)
	cmpbexadd	%ebx, %ecx, -512(%rdx)	 #CMPCCXADD Disp32(00feffff)
	cmpbexadd	%rax, %rcx, 0x10000000(%rbp, %r14, 8)	 #CMPCCXADD
	cmpbexadd	%rbx, %rcx, (%r9)	 #CMPCCXADD
	cmpbexadd	%rax, %rcx, 1016(%rcx)	 #CMPCCXADD Disp32(f8030000)
	cmpbexadd	%rbx, %rcx, -1024(%rdx)	 #CMPCCXADD Disp32(00fcffff)
	cmpbxadd	%eax, %ecx, 0x10000000(%rbp, %r14, 8)	 #CMPCCXADD
	cmpbxadd	%ebx, %ecx, (%r9)	 #CMPCCXADD
	cmpbxadd	%eax, %ecx, 508(%rcx)	 #CMPCCXADD Disp32(fc010000)
	cmpbxadd	%ebx, %ecx, -512(%rdx)	 #CMPCCXADD Disp32(00feffff)
	cmpbxadd	%rax, %rcx, 0x10000000(%rbp, %r14, 8)	 #CMPCCXADD
	cmpbxadd	%rbx, %rcx, (%r9)	 #CMPCCXADD
	cmpbxadd	%rax, %rcx, 1016(%rcx)	 #CMPCCXADD Disp32(f8030000)
	cmpbxadd	%rbx, %rcx, -1024(%rdx)	 #CMPCCXADD Disp32(00fcffff)
	cmplexadd	%eax, %ecx, 0x10000000(%rbp, %r14, 8)	 #CMPCCXADD
	cmplexadd	%ebx, %ecx, (%r9)	 #CMPCCXADD
	cmplexadd	%eax, %ecx, 508(%rcx)	 #CMPCCXADD Disp32(fc010000)
	cmplexadd	%ebx, %ecx, -512(%rdx)	 #CMPCCXADD Disp32(00feffff)
	cmplexadd	%rax, %rcx, 0x10000000(%rbp, %r14, 8)	 #CMPCCXADD
	cmplexadd	%rbx, %rcx, (%r9)	 #CMPCCXADD
	cmplexadd	%rax, %rcx, 1016(%rcx)	 #CMPCCXADD Disp32(f8030000)
	cmplexadd	%rbx, %rcx, -1024(%rdx)	 #CMPCCXADD Disp32(00fcffff)
	cmplxadd	%eax, %ecx, 0x10000000(%rbp, %r14, 8)	 #CMPCCXADD
	cmplxadd	%ebx, %ecx, (%r9)	 #CMPCCXADD
	cmplxadd	%eax, %ecx, 508(%rcx)	 #CMPCCXADD Disp32(fc010000)
	cmplxadd	%ebx, %ecx, -512(%rdx)	 #CMPCCXADD Disp32(00feffff)
	cmplxadd	%rax, %rcx, 0x10000000(%rbp, %r14, 8)	 #CMPCCXADD
	cmplxadd	%rbx, %rcx, (%r9)	 #CMPCCXADD
	cmplxadd	%rax, %rcx, 1016(%rcx)	 #CMPCCXADD Disp32(f8030000)
	cmplxadd	%rbx, %rcx, -1024(%rdx)	 #CMPCCXADD Disp32(00fcffff)
	cmpnbexadd	%eax, %ecx, 0x10000000(%rbp, %r14, 8)	 #CMPCCXADD
	cmpnbexadd	%ebx, %ecx, (%r9)	 #CMPCCXADD
	cmpnbexadd	%eax, %ecx, 508(%rcx)	 #CMPCCXADD Disp32(fc010000)
	cmpnbexadd	%ebx, %ecx, -512(%rdx)	 #CMPCCXADD Disp32(00feffff)
	cmpnbexadd	%rax, %rcx, 0x10000000(%rbp, %r14, 8)	 #CMPCCXADD
	cmpnbexadd	%rbx, %rcx, (%r9)	 #CMPCCXADD
	cmpnbexadd	%rax, %rcx, 1016(%rcx)	 #CMPCCXADD Disp32(f8030000)
	cmpnbexadd	%rbx, %rcx, -1024(%rdx)	 #CMPCCXADD Disp32(00fcffff)
	cmpnbxadd	%eax, %ecx, 0x10000000(%rbp, %r14, 8)	 #CMPCCXADD
	cmpnbxadd	%ebx, %ecx, (%r9)	 #CMPCCXADD
	cmpnbxadd	%eax, %ecx, 508(%rcx)	 #CMPCCXADD Disp32(fc010000)
	cmpnbxadd	%ebx, %ecx, -512(%rdx)	 #CMPCCXADD Disp32(00feffff)
	cmpnbxadd	%rax, %rcx, 0x10000000(%rbp, %r14, 8)	 #CMPCCXADD
	cmpnbxadd	%rbx, %rcx, (%r9)	 #CMPCCXADD
	cmpnbxadd	%rax, %rcx, 1016(%rcx)	 #CMPCCXADD Disp32(f8030000)
	cmpnbxadd	%rbx, %rcx, -1024(%rdx)	 #CMPCCXADD Disp32(00fcffff)
	cmpnlexadd	%eax, %ecx, 0x10000000(%rbp, %r14, 8)	 #CMPCCXADD
	cmpnlexadd	%ebx, %ecx, (%r9)	 #CMPCCXADD
	cmpnlexadd	%eax, %ecx, 508(%rcx)	 #CMPCCXADD Disp32(fc010000)
	cmpnlexadd	%ebx, %ecx, -512(%rdx)	 #CMPCCXADD Disp32(00feffff)
	cmpnlexadd	%rax, %rcx, 0x10000000(%rbp, %r14, 8)	 #CMPCCXADD
	cmpnlexadd	%rbx, %rcx, (%r9)	 #CMPCCXADD
	cmpnlexadd	%rax, %rcx, 1016(%rcx)	 #CMPCCXADD Disp32(f8030000)
	cmpnlexadd	%rbx, %rcx, -1024(%rdx)	 #CMPCCXADD Disp32(00fcffff)
	cmpnlxadd	%eax, %ecx, 0x10000000(%rbp, %r14, 8)	 #CMPCCXADD
	cmpnlxadd	%ebx, %ecx, (%r9)	 #CMPCCXADD
	cmpnlxadd	%eax, %ecx, 508(%rcx)	 #CMPCCXADD Disp32(fc010000)
	cmpnlxadd	%ebx, %ecx, -512(%rdx)	 #CMPCCXADD Disp32(00feffff)
	cmpnlxadd	%rax, %rcx, 0x10000000(%rbp, %r14, 8)	 #CMPCCXADD
	cmpnlxadd	%rbx, %rcx, (%r9)	 #CMPCCXADD
	cmpnlxadd	%rax, %rcx, 1016(%rcx)	 #CMPCCXADD Disp32(f8030000)
	cmpnlxadd	%rbx, %rcx, -1024(%rdx)	 #CMPCCXADD Disp32(00fcffff)
	cmpnoxadd	%eax, %ecx, 0x10000000(%rbp, %r14, 8)	 #CMPCCXADD
	cmpnoxadd	%ebx, %ecx, (%r9)	 #CMPCCXADD
	cmpnoxadd	%eax, %ecx, 508(%rcx)	 #CMPCCXADD Disp32(fc010000)
	cmpnoxadd	%ebx, %ecx, -512(%rdx)	 #CMPCCXADD Disp32(00feffff)
	cmpnoxadd	%rax, %rcx, 0x10000000(%rbp, %r14, 8)	 #CMPCCXADD
	cmpnoxadd	%rbx, %rcx, (%r9)	 #CMPCCXADD
	cmpnoxadd	%rax, %rcx, 1016(%rcx)	 #CMPCCXADD Disp32(f8030000)
	cmpnoxadd	%rbx, %rcx, -1024(%rdx)	 #CMPCCXADD Disp32(00fcffff)
	cmpnpxadd	%eax, %ecx, 0x10000000(%rbp, %r14, 8)	 #CMPCCXADD
	cmpnpxadd	%ebx, %ecx, (%r9)	 #CMPCCXADD
	cmpnpxadd	%eax, %ecx, 508(%rcx)	 #CMPCCXADD Disp32(fc010000)
	cmpnpxadd	%ebx, %ecx, -512(%rdx)	 #CMPCCXADD Disp32(00feffff)
	cmpnpxadd	%rax, %rcx, 0x10000000(%rbp, %r14, 8)	 #CMPCCXADD
	cmpnpxadd	%rbx, %rcx, (%r9)	 #CMPCCXADD
	cmpnpxadd	%rax, %rcx, 1016(%rcx)	 #CMPCCXADD Disp32(f8030000)
	cmpnpxadd	%rbx, %rcx, -1024(%rdx)	 #CMPCCXADD Disp32(00fcffff)
	cmpnsxadd	%eax, %ecx, 0x10000000(%rbp, %r14, 8)	 #CMPCCXADD
	cmpnsxadd	%ebx, %ecx, (%r9)	 #CMPCCXADD
	cmpnsxadd	%eax, %ecx, 508(%rcx)	 #CMPCCXADD Disp32(fc010000)
	cmpnsxadd	%ebx, %ecx, -512(%rdx)	 #CMPCCXADD Disp32(00feffff)
	cmpnsxadd	%rax, %rcx, 0x10000000(%rbp, %r14, 8)	 #CMPCCXADD
	cmpnsxadd	%rbx, %rcx, (%r9)	 #CMPCCXADD
	cmpnsxadd	%rax, %rcx, 1016(%rcx)	 #CMPCCXADD Disp32(f8030000)
	cmpnsxadd	%rbx, %rcx, -1024(%rdx)	 #CMPCCXADD Disp32(00fcffff)
	cmpnzxadd	%eax, %ecx, 0x10000000(%rbp, %r14, 8)	 #CMPCCXADD
	cmpnzxadd	%ebx, %ecx, (%r9)	 #CMPCCXADD
	cmpnzxadd	%eax, %ecx, 508(%rcx)	 #CMPCCXADD Disp32(fc010000)
	cmpnzxadd	%ebx, %ecx, -512(%rdx)	 #CMPCCXADD Disp32(00feffff)
	cmpnzxadd	%rax, %rcx, 0x10000000(%rbp, %r14, 8)	 #CMPCCXADD
	cmpnzxadd	%rbx, %rcx, (%r9)	 #CMPCCXADD
	cmpnzxadd	%rax, %rcx, 1016(%rcx)	 #CMPCCXADD Disp32(f8030000)
	cmpnzxadd	%rbx, %rcx, -1024(%rdx)	 #CMPCCXADD Disp32(00fcffff)
	cmpoxadd	%eax, %ecx, 0x10000000(%rbp, %r14, 8)	 #CMPCCXADD
	cmpoxadd	%ebx, %ecx, (%r9)	 #CMPCCXADD
	cmpoxadd	%eax, %ecx, 508(%rcx)	 #CMPCCXADD Disp32(fc010000)
	cmpoxadd	%ebx, %ecx, -512(%rdx)	 #CMPCCXADD Disp32(00feffff)
	cmpoxadd	%rax, %rcx, 0x10000000(%rbp, %r14, 8)	 #CMPCCXADD
	cmpoxadd	%rbx, %rcx, (%r9)	 #CMPCCXADD
	cmpoxadd	%rax, %rcx, 1016(%rcx)	 #CMPCCXADD Disp32(f8030000)
	cmpoxadd	%rbx, %rcx, -1024(%rdx)	 #CMPCCXADD Disp32(00fcffff)
	cmppxadd	%eax, %ecx, 0x10000000(%rbp, %r14, 8)	 #CMPCCXADD
	cmppxadd	%ebx, %ecx, (%r9)	 #CMPCCXADD
	cmppxadd	%eax, %ecx, 508(%rcx)	 #CMPCCXADD Disp32(fc010000)
	cmppxadd	%ebx, %ecx, -512(%rdx)	 #CMPCCXADD Disp32(00feffff)
	cmppxadd	%rax, %rcx, 0x10000000(%rbp, %r14, 8)	 #CMPCCXADD
	cmppxadd	%rbx, %rcx, (%r9)	 #CMPCCXADD
	cmppxadd	%rax, %rcx, 1016(%rcx)	 #CMPCCXADD Disp32(f8030000)
	cmppxadd	%rbx, %rcx, -1024(%rdx)	 #CMPCCXADD Disp32(00fcffff)
	cmpsxadd	%eax, %ecx, 0x10000000(%rbp, %r14, 8)	 #CMPCCXADD
	cmpsxadd	%ebx, %ecx, (%r9)	 #CMPCCXADD
	cmpsxadd	%eax, %ecx, 508(%rcx)	 #CMPCCXADD Disp32(fc010000)
	cmpsxadd	%ebx, %ecx, -512(%rdx)	 #CMPCCXADD Disp32(00feffff)
	cmpsxadd	%rax, %rcx, 0x10000000(%rbp, %r14, 8)	 #CMPCCXADD
	cmpsxadd	%rbx, %rcx, (%r9)	 #CMPCCXADD
	cmpsxadd	%rax, %rcx, 1016(%rcx)	 #CMPCCXADD Disp32(f8030000)
	cmpsxadd	%rbx, %rcx, -1024(%rdx)	 #CMPCCXADD Disp32(00fcffff)
	cmpzxadd	%eax, %ecx, 0x10000000(%rbp, %r14, 8)	 #CMPCCXADD
	cmpzxadd	%ebx, %ecx, (%r9)	 #CMPCCXADD
	cmpzxadd	%eax, %ecx, 508(%rcx)	 #CMPCCXADD Disp32(fc010000)
	cmpzxadd	%ebx, %ecx, -512(%rdx)	 #CMPCCXADD Disp32(00feffff)
	cmpzxadd	%rax, %rcx, 0x10000000(%rbp, %r14, 8)	 #CMPCCXADD
	cmpzxadd	%rbx, %rcx, (%r9)	 #CMPCCXADD
	cmpzxadd	%rax, %rcx, 1016(%rcx)	 #CMPCCXADD Disp32(f8030000)
	cmpzxadd	%rbx, %rcx, -1024(%rdx)	 #CMPCCXADD Disp32(00fcffff)

.intel_syntax noprefix
	cmpbexadd	DWORD PTR [rbp+r14*8+0x10000000], ecx, eax	 #CMPCCXADD
	cmpbexadd	DWORD PTR [r9], ecx, ebx	 #CMPCCXADD
	cmpbexadd	DWORD PTR [rcx+508], ecx, eax	 #CMPCCXADD Disp32(fc010000)
	cmpbexadd	DWORD PTR [rdx-512], ecx, ebx	 #CMPCCXADD Disp32(00feffff)
	cmpbexadd	QWORD PTR [rbp+r14*8+0x10000000], rcx, rax	 #CMPCCXADD
	cmpbexadd	QWORD PTR [r9], rcx, rbx	 #CMPCCXADD
	cmpbexadd	QWORD PTR [rcx+1016], rcx, rax	 #CMPCCXADD Disp32(f8030000)
	cmpbexadd	QWORD PTR [rdx-1024], rcx, rbx	 #CMPCCXADD Disp32(00fcffff)
	cmpbxadd	DWORD PTR [rbp+r14*8+0x10000000], ecx, eax	 #CMPCCXADD
	cmpbxadd	DWORD PTR [r9], ecx, ebx	 #CMPCCXADD
	cmpbxadd	DWORD PTR [rcx+508], ecx, eax	 #CMPCCXADD Disp32(fc010000)
	cmpbxadd	DWORD PTR [rdx-512], ecx, ebx	 #CMPCCXADD Disp32(00feffff)
	cmpbxadd	QWORD PTR [rbp+r14*8+0x10000000], rcx, rax	 #CMPCCXADD
	cmpbxadd	QWORD PTR [r9], rcx, rbx	 #CMPCCXADD
	cmpbxadd	QWORD PTR [rcx+1016], rcx, rax	 #CMPCCXADD Disp32(f8030000)
	cmpbxadd	QWORD PTR [rdx-1024], rcx, rbx	 #CMPCCXADD Disp32(00fcffff)
	cmplexadd	DWORD PTR [rbp+r14*8+0x10000000], ecx, eax	 #CMPCCXADD
	cmplexadd	DWORD PTR [r9], ecx, ebx	 #CMPCCXADD
	cmplexadd	DWORD PTR [rcx+508], ecx, eax	 #CMPCCXADD Disp32(fc010000)
	cmplexadd	DWORD PTR [rdx-512], ecx, ebx	 #CMPCCXADD Disp32(00feffff)
	cmplexadd	QWORD PTR [rbp+r14*8+0x10000000], rcx, rax	 #CMPCCXADD
	cmplexadd	QWORD PTR [r9], rcx, rbx	 #CMPCCXADD
	cmplexadd	QWORD PTR [rcx+1016], rcx, rax	 #CMPCCXADD Disp32(f8030000)
	cmplexadd	QWORD PTR [rdx-1024], rcx, rbx	 #CMPCCXADD Disp32(00fcffff)
	cmplxadd	DWORD PTR [rbp+r14*8+0x10000000], ecx, eax	 #CMPCCXADD
	cmplxadd	DWORD PTR [r9], ecx, ebx	 #CMPCCXADD
	cmplxadd	DWORD PTR [rcx+508], ecx, eax	 #CMPCCXADD Disp32(fc010000)
	cmplxadd	DWORD PTR [rdx-512], ecx, ebx	 #CMPCCXADD Disp32(00feffff)
	cmplxadd	QWORD PTR [rbp+r14*8+0x10000000], rcx, rax	 #CMPCCXADD
	cmplxadd	QWORD PTR [r9], rcx, rbx	 #CMPCCXADD
	cmplxadd	QWORD PTR [rcx+1016], rcx, rax	 #CMPCCXADD Disp32(f8030000)
	cmplxadd	QWORD PTR [rdx-1024], rcx, rbx	 #CMPCCXADD Disp32(00fcffff)
	cmpnbexadd	DWORD PTR [rbp+r14*8+0x10000000], ecx, eax	 #CMPCCXADD
	cmpnbexadd	DWORD PTR [r9], ecx, ebx	 #CMPCCXADD
	cmpnbexadd	DWORD PTR [rcx+508], ecx, eax	 #CMPCCXADD Disp32(fc010000)
	cmpnbexadd	DWORD PTR [rdx-512], ecx, ebx	 #CMPCCXADD Disp32(00feffff)
	cmpnbexadd	QWORD PTR [rbp+r14*8+0x10000000], rcx, rax	 #CMPCCXADD
	cmpnbexadd	QWORD PTR [r9], rcx, rbx	 #CMPCCXADD
	cmpnbexadd	QWORD PTR [rcx+1016], rcx, rax	 #CMPCCXADD Disp32(f8030000)
	cmpnbexadd	QWORD PTR [rdx-1024], rcx, rbx	 #CMPCCXADD Disp32(00fcffff)
	cmpnbxadd	DWORD PTR [rbp+r14*8+0x10000000], ecx, eax	 #CMPCCXADD
	cmpnbxadd	DWORD PTR [r9], ecx, ebx	 #CMPCCXADD
	cmpnbxadd	DWORD PTR [rcx+508], ecx, eax	 #CMPCCXADD Disp32(fc010000)
	cmpnbxadd	DWORD PTR [rdx-512], ecx, ebx	 #CMPCCXADD Disp32(00feffff)
	cmpnbxadd	QWORD PTR [rbp+r14*8+0x10000000], rcx, rax	 #CMPCCXADD
	cmpnbxadd	QWORD PTR [r9], rcx, rbx	 #CMPCCXADD
	cmpnbxadd	QWORD PTR [rcx+1016], rcx, rax	 #CMPCCXADD Disp32(f8030000)
	cmpnbxadd	QWORD PTR [rdx-1024], rcx, rbx	 #CMPCCXADD Disp32(00fcffff)
	cmpnlexadd	DWORD PTR [rbp+r14*8+0x10000000], ecx, eax	 #CMPCCXADD
	cmpnlexadd	DWORD PTR [r9], ecx, ebx	 #CMPCCXADD
	cmpnlexadd	DWORD PTR [rcx+508], ecx, eax	 #CMPCCXADD Disp32(fc010000)
	cmpnlexadd	DWORD PTR [rdx-512], ecx, ebx	 #CMPCCXADD Disp32(00feffff)
	cmpnlexadd	QWORD PTR [rbp+r14*8+0x10000000], rcx, rax	 #CMPCCXADD
	cmpnlexadd	QWORD PTR [r9], rcx, rbx	 #CMPCCXADD
	cmpnlexadd	QWORD PTR [rcx+1016], rcx, rax	 #CMPCCXADD Disp32(f8030000)
	cmpnlexadd	QWORD PTR [rdx-1024], rcx, rbx	 #CMPCCXADD Disp32(00fcffff)
	cmpnlxadd	DWORD PTR [rbp+r14*8+0x10000000], ecx, eax	 #CMPCCXADD
	cmpnlxadd	DWORD PTR [r9], ecx, ebx	 #CMPCCXADD
	cmpnlxadd	DWORD PTR [rcx+508], ecx, eax	 #CMPCCXADD Disp32(fc010000)
	cmpnlxadd	DWORD PTR [rdx-512], ecx, ebx	 #CMPCCXADD Disp32(00feffff)
	cmpnlxadd	QWORD PTR [rbp+r14*8+0x10000000], rcx, rax	 #CMPCCXADD
	cmpnlxadd	QWORD PTR [r9], rcx, rbx	 #CMPCCXADD
	cmpnlxadd	QWORD PTR [rcx+1016], rcx, rax	 #CMPCCXADD Disp32(f8030000)
	cmpnlxadd	QWORD PTR [rdx-1024], rcx, rbx	 #CMPCCXADD Disp32(00fcffff)
	cmpnoxadd	DWORD PTR [rbp+r14*8+0x10000000], ecx, eax	 #CMPCCXADD
	cmpnoxadd	DWORD PTR [r9], ecx, ebx	 #CMPCCXADD
	cmpnoxadd	DWORD PTR [rcx+508], ecx, eax	 #CMPCCXADD Disp32(fc010000)
	cmpnoxadd	DWORD PTR [rdx-512], ecx, ebx	 #CMPCCXADD Disp32(00feffff)
	cmpnoxadd	QWORD PTR [rbp+r14*8+0x10000000], rcx, rax	 #CMPCCXADD
	cmpnoxadd	QWORD PTR [r9], rcx, rbx	 #CMPCCXADD
	cmpnoxadd	QWORD PTR [rcx+1016], rcx, rax	 #CMPCCXADD Disp32(f8030000)
	cmpnoxadd	QWORD PTR [rdx-1024], rcx, rbx	 #CMPCCXADD Disp32(00fcffff)
	cmpnpxadd	DWORD PTR [rbp+r14*8+0x10000000], ecx, eax	 #CMPCCXADD
	cmpnpxadd	DWORD PTR [r9], ecx, ebx	 #CMPCCXADD
	cmpnpxadd	DWORD PTR [rcx+508], ecx, eax	 #CMPCCXADD Disp32(fc010000)
	cmpnpxadd	DWORD PTR [rdx-512], ecx, ebx	 #CMPCCXADD Disp32(00feffff)
	cmpnpxadd	QWORD PTR [rbp+r14*8+0x10000000], rcx, rax	 #CMPCCXADD
	cmpnpxadd	QWORD PTR [r9], rcx, rbx	 #CMPCCXADD
	cmpnpxadd	QWORD PTR [rcx+1016], rcx, rax	 #CMPCCXADD Disp32(f8030000)
	cmpnpxadd	QWORD PTR [rdx-1024], rcx, rbx	 #CMPCCXADD Disp32(00fcffff)
	cmpnsxadd	DWORD PTR [rbp+r14*8+0x10000000], ecx, eax	 #CMPCCXADD
	cmpnsxadd	DWORD PTR [r9], ecx, ebx	 #CMPCCXADD
	cmpnsxadd	DWORD PTR [rcx+508], ecx, eax	 #CMPCCXADD Disp32(fc010000)
	cmpnsxadd	DWORD PTR [rdx-512], ecx, ebx	 #CMPCCXADD Disp32(00feffff)
	cmpnsxadd	QWORD PTR [rbp+r14*8+0x10000000], rcx, rax	 #CMPCCXADD
	cmpnsxadd	QWORD PTR [r9], rcx, rbx	 #CMPCCXADD
	cmpnsxadd	QWORD PTR [rcx+1016], rcx, rax	 #CMPCCXADD Disp32(f8030000)
	cmpnsxadd	QWORD PTR [rdx-1024], rcx, rbx	 #CMPCCXADD Disp32(00fcffff)
	cmpnzxadd	DWORD PTR [rbp+r14*8+0x10000000], ecx, eax	 #CMPCCXADD
	cmpnzxadd	DWORD PTR [r9], ecx, ebx	 #CMPCCXADD
	cmpnzxadd	DWORD PTR [rcx+508], ecx, eax	 #CMPCCXADD Disp32(fc010000)
	cmpnzxadd	DWORD PTR [rdx-512], ecx, ebx	 #CMPCCXADD Disp32(00feffff)
	cmpnzxadd	QWORD PTR [rbp+r14*8+0x10000000], rcx, rax	 #CMPCCXADD
	cmpnzxadd	QWORD PTR [r9], rcx, rbx	 #CMPCCXADD
	cmpnzxadd	QWORD PTR [rcx+1016], rcx, rax	 #CMPCCXADD Disp32(f8030000)
	cmpnzxadd	QWORD PTR [rdx-1024], rcx, rbx	 #CMPCCXADD Disp32(00fcffff)
	cmpoxadd	DWORD PTR [rbp+r14*8+0x10000000], ecx, eax	 #CMPCCXADD
	cmpoxadd	DWORD PTR [r9], ecx, ebx	 #CMPCCXADD
	cmpoxadd	DWORD PTR [rcx+508], ecx, eax	 #CMPCCXADD Disp32(fc010000)
	cmpoxadd	DWORD PTR [rdx-512], ecx, ebx	 #CMPCCXADD Disp32(00feffff)
	cmpoxadd	QWORD PTR [rbp+r14*8+0x10000000], rcx, rax	 #CMPCCXADD
	cmpoxadd	QWORD PTR [r9], rcx, rbx	 #CMPCCXADD
	cmpoxadd	QWORD PTR [rcx+1016], rcx, rax	 #CMPCCXADD Disp32(f8030000)
	cmpoxadd	QWORD PTR [rdx-1024], rcx, rbx	 #CMPCCXADD Disp32(00fcffff)
	cmppxadd	DWORD PTR [rbp+r14*8+0x10000000], ecx, eax	 #CMPCCXADD
	cmppxadd	DWORD PTR [r9], ecx, ebx	 #CMPCCXADD
	cmppxadd	DWORD PTR [rcx+508], ecx, eax	 #CMPCCXADD Disp32(fc010000)
	cmppxadd	DWORD PTR [rdx-512], ecx, ebx	 #CMPCCXADD Disp32(00feffff)
	cmppxadd	QWORD PTR [rbp+r14*8+0x10000000], rcx, rax	 #CMPCCXADD
	cmppxadd	QWORD PTR [r9], rcx, rbx	 #CMPCCXADD
	cmppxadd	QWORD PTR [rcx+1016], rcx, rax	 #CMPCCXADD Disp32(f8030000)
	cmppxadd	QWORD PTR [rdx-1024], rcx, rbx	 #CMPCCXADD Disp32(00fcffff)
	cmpsxadd	DWORD PTR [rbp+r14*8+0x10000000], ecx, eax	 #CMPCCXADD
	cmpsxadd	DWORD PTR [r9], ecx, ebx	 #CMPCCXADD
	cmpsxadd	DWORD PTR [rcx+508], ecx, eax	 #CMPCCXADD Disp32(fc010000)
	cmpsxadd	DWORD PTR [rdx-512], ecx, ebx	 #CMPCCXADD Disp32(00feffff)
	cmpsxadd	QWORD PTR [rbp+r14*8+0x10000000], rcx, rax	 #CMPCCXADD
	cmpsxadd	QWORD PTR [r9], rcx, rbx	 #CMPCCXADD
	cmpsxadd	QWORD PTR [rcx+1016], rcx, rax	 #CMPCCXADD Disp32(f8030000)
	cmpsxadd	QWORD PTR [rdx-1024], rcx, rbx	 #CMPCCXADD Disp32(00fcffff)
	cmpzxadd	DWORD PTR [rbp+r14*8+0x10000000], ecx, eax	 #CMPCCXADD
	cmpzxadd	DWORD PTR [r9], ecx, ebx	 #CMPCCXADD
	cmpzxadd	DWORD PTR [rcx+508], ecx, eax	 #CMPCCXADD Disp32(fc010000)
	cmpzxadd	DWORD PTR [rdx-512], ecx, ebx	 #CMPCCXADD Disp32(00feffff)
	cmpzxadd	QWORD PTR [rbp+r14*8+0x10000000], rcx, rax	 #CMPCCXADD
	cmpzxadd	QWORD PTR [r9], rcx, rbx	 #CMPCCXADD
	cmpzxadd	QWORD PTR [rcx+1016], rcx, rax	 #CMPCCXADD Disp32(f8030000)
	cmpzxadd	QWORD PTR [rdx-1024], rcx, rbx	 #CMPCCXADD Disp32(00fcffff)
