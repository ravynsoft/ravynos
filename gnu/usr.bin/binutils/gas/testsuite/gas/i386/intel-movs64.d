#objdump: -dMintel
#source: intel-movs.s
#name: x86 Intel movs (64-bit object)

.*: +file format .*

Disassembly of section .text:

0+ <movs>:
[ 	]*[a-f0-9]+:	a4 *	movs(b *| +BYTE PTR es:\[rdi\],(BYTE PTR )?(ds:)?\[rsi\])
[ 	]*[a-f0-9]+:	67 a4 *	movs +BYTE PTR es:\[edi\],(BYTE PTR )?(ds:)?\[esi\]
[ 	]*[a-f0-9]+:	64 67 a4 *	movs +BYTE PTR es:\[edi\],(BYTE PTR )?fs:\[esi\]
[ 	]*[a-f0-9]+:	67 a4 *	movs +BYTE PTR es:\[edi\],(BYTE PTR )?(ds:)?\[esi\]
[ 	]*[a-f0-9]+:	67 a4 *	movs +BYTE PTR es:\[edi\],(BYTE PTR )?(ds:)?\[esi\]
[ 	]*[a-f0-9]+:	67 a4 *	movs +BYTE PTR es:\[edi\],(BYTE PTR )?(ds:)?\[esi\]
[ 	]*[a-f0-9]+:	67 a4 *	movs +BYTE PTR es:\[edi\],(BYTE PTR )?(ds:)?\[esi\]
[ 	]*[a-f0-9]+:	67 a4 *	movs +BYTE PTR es:\[edi\],(BYTE PTR )?(ds:)?\[esi\]
[ 	]*[a-f0-9]+:	67 a4 *	movs +BYTE PTR es:\[edi\],(BYTE PTR )?(ds:)?\[esi\]
[ 	]*[a-f0-9]+:	67 a4 *	movs +BYTE PTR es:\[edi\],(BYTE PTR )?(ds:)?\[esi\]
[ 	]*[a-f0-9]+:	a4 *	movs(b *| +BYTE PTR es:\[rdi\],(BYTE PTR )?(ds:)?\[rsi\])
[ 	]*[a-f0-9]+:	64 a4 *	movs +BYTE PTR es:\[rdi\],(BYTE PTR )?fs:\[rsi\]
[ 	]*[a-f0-9]+:	a4 *	movs(b *| +BYTE PTR es:\[rdi\],(BYTE PTR )?(ds:)?\[rsi\])
[ 	]*[a-f0-9]+:	a4 *	movs(b *| +BYTE PTR es:\[rdi\],(BYTE PTR )?(ds:)?\[rsi\])
[ 	]*[a-f0-9]+:	a4 *	movs(b *| +BYTE PTR es:\[rdi\],(BYTE PTR )?(ds:)?\[rsi\])
[ 	]*[a-f0-9]+:	a4 *	movs(b *| +BYTE PTR es:\[rdi\],(BYTE PTR )?(ds:)?\[rsi\])
[ 	]*[a-f0-9]+:	a4 *	movs(b *| +BYTE PTR es:\[rdi\],(BYTE PTR )?(ds:)?\[rsi\])
[ 	]*[a-f0-9]+:	a4 *	movs(b *| +BYTE PTR es:\[rdi\],(BYTE PTR )?(ds:)?\[rsi\])
[ 	]*[a-f0-9]+:	a4 *	movs(b *| +BYTE PTR es:\[rdi\],(BYTE PTR )?(ds:)?\[rsi\])
[ 	]*[a-f0-9]+:	66 a5 *	movs(w *| +WORD PTR es:\[rdi\],(WORD PTR )?(ds:)?\[rsi\])
[ 	]*[a-f0-9]+:	67 66 a5 *	movs +WORD PTR es:\[edi\],(WORD PTR )?(ds:)?\[esi\]
[ 	]*[a-f0-9]+:	64 67 66 a5 *	movs +WORD PTR es:\[edi\],(WORD PTR )?fs:\[esi\]
[ 	]*[a-f0-9]+:	67 66 a5 *	movs +WORD PTR es:\[edi\],(WORD PTR )?(ds:)?\[esi\]
[ 	]*[a-f0-9]+:	67 66 a5 *	movs +WORD PTR es:\[edi\],(WORD PTR )?(ds:)?\[esi\]
[ 	]*[a-f0-9]+:	67 66 a5 *	movs +WORD PTR es:\[edi\],(WORD PTR )?(ds:)?\[esi\]
[ 	]*[a-f0-9]+:	67 66 a5 *	movs +WORD PTR es:\[edi\],(WORD PTR )?(ds:)?\[esi\]
[ 	]*[a-f0-9]+:	67 66 a5 *	movs +WORD PTR es:\[edi\],(WORD PTR )?(ds:)?\[esi\]
[ 	]*[a-f0-9]+:	67 66 a5 *	movs +WORD PTR es:\[edi\],(WORD PTR )?(ds:)?\[esi\]
[ 	]*[a-f0-9]+:	67 66 a5 *	movs +WORD PTR es:\[edi\],(WORD PTR )?(ds:)?\[esi\]
[ 	]*[a-f0-9]+:	66 a5 *	movs(w *| +WORD PTR es:\[rdi\],(WORD PTR )?(ds:)?\[rsi\])
[ 	]*[a-f0-9]+:	64 66 a5 *	movs +WORD PTR es:\[rdi\],(WORD PTR )?fs:\[rsi\]
[ 	]*[a-f0-9]+:	66 a5 *	movs(w *| +WORD PTR es:\[rdi\],(WORD PTR )?(ds:)?\[rsi\])
[ 	]*[a-f0-9]+:	66 a5 *	movs(w *| +WORD PTR es:\[rdi\],(WORD PTR )?(ds:)?\[rsi\])
[ 	]*[a-f0-9]+:	66 a5 *	movs(w *| +WORD PTR es:\[rdi\],(WORD PTR )?(ds:)?\[rsi\])
[ 	]*[a-f0-9]+:	66 a5 *	movs(w *| +WORD PTR es:\[rdi\],(WORD PTR )?(ds:)?\[rsi\])
[ 	]*[a-f0-9]+:	66 a5 *	movs(w *| +WORD PTR es:\[rdi\],(WORD PTR )?(ds:)?\[rsi\])
[ 	]*[a-f0-9]+:	66 a5 *	movs(w *| +WORD PTR es:\[rdi\],(WORD PTR )?(ds:)?\[rsi\])
[ 	]*[a-f0-9]+:	66 a5 *	movs(w *| +WORD PTR es:\[rdi\],(WORD PTR )?(ds:)?\[rsi\])
[ 	]*[a-f0-9]+:	a5 *	movs(d *| +DWORD PTR es:\[rdi\],(DWORD PTR )?(ds:)?\[rsi\])
[ 	]*[a-f0-9]+:	67 a5 *	movs +DWORD PTR es:\[edi\],(DWORD PTR )?(ds:)?\[esi\]
[ 	]*[a-f0-9]+:	64 67 a5 *	movs +DWORD PTR es:\[edi\],(DWORD PTR )?fs:\[esi\]
[ 	]*[a-f0-9]+:	67 a5 *	movs +DWORD PTR es:\[edi\],(DWORD PTR )?(ds:)?\[esi\]
[ 	]*[a-f0-9]+:	67 a5 *	movs +DWORD PTR es:\[edi\],(DWORD PTR )?(ds:)?\[esi\]
[ 	]*[a-f0-9]+:	67 a5 *	movs +DWORD PTR es:\[edi\],(DWORD PTR )?(ds:)?\[esi\]
[ 	]*[a-f0-9]+:	67 a5 *	movs +DWORD PTR es:\[edi\],(DWORD PTR )?(ds:)?\[esi\]
[ 	]*[a-f0-9]+:	67 a5 *	movs +DWORD PTR es:\[edi\],(DWORD PTR )?(ds:)?\[esi\]
[ 	]*[a-f0-9]+:	67 a5 *	movs +DWORD PTR es:\[edi\],(DWORD PTR )?(ds:)?\[esi\]
[ 	]*[a-f0-9]+:	67 a5 *	movs +DWORD PTR es:\[edi\],(DWORD PTR )?(ds:)?\[esi\]
[ 	]*[a-f0-9]+:	a5 *	movs(d *| +DWORD PTR es:\[rdi\],(DWORD PTR )?(ds:)?\[rsi\])
[ 	]*[a-f0-9]+:	64 a5 *	movs +DWORD PTR es:\[rdi\],(DWORD PTR )?fs:\[rsi\]
[ 	]*[a-f0-9]+:	a5 *	movs(d *| +DWORD PTR es:\[rdi\],(DWORD PTR )?(ds:)?\[rsi\])
[ 	]*[a-f0-9]+:	a5 *	movs(d *| +DWORD PTR es:\[rdi\],(DWORD PTR )?(ds:)?\[rsi\])
[ 	]*[a-f0-9]+:	a5 *	movs(d *| +DWORD PTR es:\[rdi\],(DWORD PTR )?(ds:)?\[rsi\])
[ 	]*[a-f0-9]+:	a5 *	movs(d *| +DWORD PTR es:\[rdi\],(DWORD PTR )?(ds:)?\[rsi\])
[ 	]*[a-f0-9]+:	a5 *	movs(d *| +DWORD PTR es:\[rdi\],(DWORD PTR )?(ds:)?\[rsi\])
[ 	]*[a-f0-9]+:	a5 *	movs(d *| +DWORD PTR es:\[rdi\],(DWORD PTR )?(ds:)?\[rsi\])
[ 	]*[a-f0-9]+:	a5 *	movs(d *| +DWORD PTR es:\[rdi\],(DWORD PTR )?(ds:)?\[rsi\])
[ 	]*[a-f0-9]+:	48 a5 *	movs(q *| +QWORD PTR es:\[rdi\],(QWORD PTR )?(ds:)?\[rsi\])
[ 	]*[a-f0-9]+:	48 a5 *	movs(q *| +QWORD PTR es:\[rdi\],(QWORD PTR )?(ds:)?\[rsi\])
[ 	]*[a-f0-9]+:	64 48 a5 *	movs +QWORD PTR es:\[rdi\],(QWORD PTR )?fs:?\[rsi\]
[ 	]*[a-f0-9]+:	48 a5 *	movs(q *| +QWORD PTR es:\[rdi\],(QWORD PTR )?(ds:)?\[rsi\])
[ 	]*[a-f0-9]+:	48 a5 *	movs(q *| +QWORD PTR es:\[rdi\],(QWORD PTR )?(ds:)?\[rsi\])
[ 	]*[a-f0-9]+:	48 a5 *	movs(q *| +QWORD PTR es:\[rdi\],(QWORD PTR )?(ds:)?\[rsi\])
[ 	]*[a-f0-9]+:	48 a5 *	movs(q *| +QWORD PTR es:\[rdi\],(QWORD PTR )?(ds:)?\[rsi\])
[ 	]*[a-f0-9]+:	48 a5 *	movs(q *| +QWORD PTR es:\[rdi\],(QWORD PTR )?(ds:)?\[rsi\])
[ 	]*[a-f0-9]+:	48 a5 *	movs(q *| +QWORD PTR es:\[rdi\],(QWORD PTR )?(ds:)?\[rsi\])
[ 	]*[a-f0-9]+:	48 a5 *	movs(q *| +QWORD PTR es:\[rdi\],(QWORD PTR )?(ds:)?\[rsi\])
[ 	]*[a-f0-9]+:	67 48 a5 *	movs +QWORD PTR es:\[edi\],(QWORD PTR )?(ds:)?\[esi\]
[ 	]*[a-f0-9]+:	64 67 48 a5 *	movs +QWORD PTR es:\[edi\],(QWORD PTR )?fs:?\[esi\]
[ 	]*[a-f0-9]+:	67 48 a5 *	movs +QWORD PTR es:\[edi\],(QWORD PTR )?(ds:)?\[esi\]
[ 	]*[a-f0-9]+:	67 48 a5 *	movs +QWORD PTR es:\[edi\],(QWORD PTR )?(ds:)?\[esi\]
[ 	]*[a-f0-9]+:	67 48 a5 *	movs +QWORD PTR es:\[edi\],(QWORD PTR )?(ds:)?\[esi\]
[ 	]*[a-f0-9]+:	67 48 a5 *	movs +QWORD PTR es:\[edi\],(QWORD PTR )?(ds:)?\[esi\]
[ 	]*[a-f0-9]+:	67 48 a5 *	movs +QWORD PTR es:\[edi\],(QWORD PTR )?(ds:)?\[esi\]
[ 	]*[a-f0-9]+:	67 48 a5 *	movs +QWORD PTR es:\[edi\],(QWORD PTR )?(ds:)?\[esi\]
[ 	]*[a-f0-9]+:	67 48 a5 *	movs +QWORD PTR es:\[edi\],(QWORD PTR )?(ds:)?\[esi\]
#pass
