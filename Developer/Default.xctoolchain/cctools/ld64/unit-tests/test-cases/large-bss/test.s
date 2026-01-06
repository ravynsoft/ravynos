
#if __x86_64__


.lcomm _mediumarray1,4000,5
.lcomm _bigarray1,2000000000,5
.lcomm _bigarray2,2000000000,5
.lcomm _bigarray3,2000000000,5
.lcomm _small1,4,2
.lcomm _small2,4,2
.lcomm _small3,4,2
	

	.text
.globl _test
_test:
	pushq	%rbp
	movq	%rsp, %rbp
	movq	_bigarray1@GOTPCREL(%rip), %rax
	movq	_bigarray2@GOTPCREL(%rip), %rax
	movq	_bigarray3@GOTPCREL(%rip), %rax
	leaq	_small1(%rip),%rax
	leaq	_small2(%rip),%rax
	leaq	_small3(%rip),%rax
	leaq	_mediumarray1(%rip),%rax
	leave
	ret


#endif



