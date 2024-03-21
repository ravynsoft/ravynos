#objdump: -dMintel
#source: intel-cmps.s
#name: x86 Intel cmps (64-bit object)

.*: +file format .*

Disassembly of section .text:

0+ <cmps>:
[ 	]*[a-f0-9]+:	a6 *	cmps(b *| +BYTE PTR (ds:)?\[rsi\],(BYTE PTR )?es:\[rdi\])
[ 	]*[a-f0-9]+:	67 a6 *	cmps +BYTE PTR (ds:)?\[esi\],(BYTE PTR )?es:\[edi\]
[ 	]*[a-f0-9]+:	64 67 a6 *	cmps +BYTE PTR fs:\[esi\],(BYTE PTR )?es:\[edi\]
[ 	]*[a-f0-9]+:	67 a6 *	cmps +BYTE PTR (ds:)?\[esi\],(BYTE PTR )?es:\[edi\]
[ 	]*[a-f0-9]+:	67 a6 *	cmps +BYTE PTR (ds:)?\[esi\],(BYTE PTR )?es:\[edi\]
[ 	]*[a-f0-9]+:	67 a6 *	cmps +BYTE PTR (ds:)?\[esi\],(BYTE PTR )?es:\[edi\]
[ 	]*[a-f0-9]+:	67 a6 *	cmps +BYTE PTR (ds:)?\[esi\],(BYTE PTR )?es:\[edi\]
[ 	]*[a-f0-9]+:	67 a6 *	cmps +BYTE PTR (ds:)?\[esi\],(BYTE PTR )?es:\[edi\]
[ 	]*[a-f0-9]+:	67 a6 *	cmps +BYTE PTR (ds:)?\[esi\],(BYTE PTR )?es:\[edi\]
[ 	]*[a-f0-9]+:	67 a6 *	cmps +BYTE PTR (ds:)?\[esi\],(BYTE PTR )?es:\[edi\]
[ 	]*[a-f0-9]+:	a6 *	cmps(b *| +BYTE PTR (ds:)?\[rsi\],(BYTE PTR )?es:\[rdi\])
[ 	]*[a-f0-9]+:	64 a6 *	cmps +BYTE PTR fs:\[rsi\],(BYTE PTR )?es:\[rdi\]
[ 	]*[a-f0-9]+:	a6 *	cmps(b *| +BYTE PTR (ds:)?\[rsi\],(BYTE PTR )?es:\[rdi\])
[ 	]*[a-f0-9]+:	a6 *	cmps(b *| +BYTE PTR (ds:)?\[rsi\],(BYTE PTR )?es:\[rdi\])
[ 	]*[a-f0-9]+:	a6 *	cmps(b *| +BYTE PTR (ds:)?\[rsi\],(BYTE PTR )?es:\[rdi\])
[ 	]*[a-f0-9]+:	a6 *	cmps(b *| +BYTE PTR (ds:)?\[rsi\],(BYTE PTR )?es:\[rdi\])
[ 	]*[a-f0-9]+:	a6 *	cmps(b *| +BYTE PTR (ds:)?\[rsi\],(BYTE PTR )?es:\[rdi\])
[ 	]*[a-f0-9]+:	a6 *	cmps(b *| +BYTE PTR (ds:)?\[rsi\],(BYTE PTR )?es:\[rdi\])
[ 	]*[a-f0-9]+:	a6 *	cmps(b *| +BYTE PTR (ds:)?\[rsi\],(BYTE PTR )?es:\[rdi\])
[ 	]*[a-f0-9]+:	66 a7 *	cmps(w *| +WORD PTR (ds:)?\[rsi\],(WORD PTR )?es:\[rdi\])
[ 	]*[a-f0-9]+:	67 66 a7 *	cmps +WORD PTR (ds:)?\[esi\],(WORD PTR )?es:\[edi\]
[ 	]*[a-f0-9]+:	64 67 66 a7 *	cmps +WORD PTR fs:\[esi\],(WORD PTR )?es:\[edi\]
[ 	]*[a-f0-9]+:	67 66 a7 *	cmps +WORD PTR (ds:)?\[esi\],(WORD PTR )?es:\[edi\]
[ 	]*[a-f0-9]+:	67 66 a7 *	cmps +WORD PTR (ds:)?\[esi\],(WORD PTR )?es:\[edi\]
[ 	]*[a-f0-9]+:	67 66 a7 *	cmps +WORD PTR (ds:)?\[esi\],(WORD PTR )?es:\[edi\]
[ 	]*[a-f0-9]+:	67 66 a7 *	cmps +WORD PTR (ds:)?\[esi\],(WORD PTR )?es:\[edi\]
[ 	]*[a-f0-9]+:	67 66 a7 *	cmps +WORD PTR (ds:)?\[esi\],(WORD PTR )?es:\[edi\]
[ 	]*[a-f0-9]+:	67 66 a7 *	cmps +WORD PTR (ds:)?\[esi\],(WORD PTR )?es:\[edi\]
[ 	]*[a-f0-9]+:	67 66 a7 *	cmps +WORD PTR (ds:)?\[esi\],(WORD PTR )?es:\[edi\]
[ 	]*[a-f0-9]+:	66 a7 *	cmps(w *| +WORD PTR (ds:)?\[rsi\],(WORD PTR )?es:\[rdi\])
[ 	]*[a-f0-9]+:	64 66 a7 *	cmps +WORD PTR fs:\[rsi\],(WORD PTR )?es:\[rdi\]
[ 	]*[a-f0-9]+:	66 a7 *	cmps(w *| +WORD PTR (ds:)?\[rsi\],(WORD PTR )?es:\[rdi\])
[ 	]*[a-f0-9]+:	66 a7 *	cmps(w *| +WORD PTR (ds:)?\[rsi\],(WORD PTR )?es:\[rdi\])
[ 	]*[a-f0-9]+:	66 a7 *	cmps(w *| +WORD PTR (ds:)?\[rsi\],(WORD PTR )?es:\[rdi\])
[ 	]*[a-f0-9]+:	66 a7 *	cmps(w *| +WORD PTR (ds:)?\[rsi\],(WORD PTR )?es:\[rdi\])
[ 	]*[a-f0-9]+:	66 a7 *	cmps(w *| +WORD PTR (ds:)?\[rsi\],(WORD PTR )?es:\[rdi\])
[ 	]*[a-f0-9]+:	66 a7 *	cmps(w *| +WORD PTR (ds:)?\[rsi\],(WORD PTR )?es:\[rdi\])
[ 	]*[a-f0-9]+:	66 a7 *	cmps(w *| +WORD PTR (ds:)?\[rsi\],(WORD PTR )?es:\[rdi\])
[ 	]*[a-f0-9]+:	a7 *	cmps(d *| +DWORD PTR (ds:)?\[rsi\],(DWORD PTR )?es:\[rdi\])
[ 	]*[a-f0-9]+:	67 a7 *	cmps +DWORD PTR (ds:)?\[esi\],(DWORD PTR )?es:\[edi\]
[ 	]*[a-f0-9]+:	64 67 a7 *	cmps +DWORD PTR fs:\[esi\],(DWORD PTR )?es:\[edi\]
[ 	]*[a-f0-9]+:	67 a7 *	cmps +DWORD PTR (ds:)?\[esi\],(DWORD PTR )?es:\[edi\]
[ 	]*[a-f0-9]+:	67 a7 *	cmps +DWORD PTR (ds:)?\[esi\],(DWORD PTR )?es:\[edi\]
[ 	]*[a-f0-9]+:	67 a7 *	cmps +DWORD PTR (ds:)?\[esi\],(DWORD PTR )?es:\[edi\]
[ 	]*[a-f0-9]+:	67 a7 *	cmps +DWORD PTR (ds:)?\[esi\],(DWORD PTR )?es:\[edi\]
[ 	]*[a-f0-9]+:	67 a7 *	cmps +DWORD PTR (ds:)?\[esi\],(DWORD PTR )?es:\[edi\]
[ 	]*[a-f0-9]+:	67 a7 *	cmps +DWORD PTR (ds:)?\[esi\],(DWORD PTR )?es:\[edi\]
[ 	]*[a-f0-9]+:	67 a7 *	cmps +DWORD PTR (ds:)?\[esi\],(DWORD PTR )?es:\[edi\]
[ 	]*[a-f0-9]+:	a7 *	cmps(d *| +DWORD PTR (ds:)?\[rsi\],(DWORD PTR )?es:\[rdi\])
[ 	]*[a-f0-9]+:	64 a7 *	cmps +DWORD PTR fs:\[rsi\],(DWORD PTR )?es:\[rdi\]
[ 	]*[a-f0-9]+:	a7 *	cmps(d *| +DWORD PTR (ds:)?\[rsi\],(DWORD PTR )?es:\[rdi\])
[ 	]*[a-f0-9]+:	a7 *	cmps(d *| +DWORD PTR (ds:)?\[rsi\],(DWORD PTR )?es:\[rdi\])
[ 	]*[a-f0-9]+:	a7 *	cmps(d *| +DWORD PTR (ds:)?\[rsi\],(DWORD PTR )?es:\[rdi\])
[ 	]*[a-f0-9]+:	a7 *	cmps(d *| +DWORD PTR (ds:)?\[rsi\],(DWORD PTR )?es:\[rdi\])
[ 	]*[a-f0-9]+:	a7 *	cmps(d *| +DWORD PTR (ds:)?\[rsi\],(DWORD PTR )?es:\[rdi\])
[ 	]*[a-f0-9]+:	a7 *	cmps(d *| +DWORD PTR (ds:)?\[rsi\],(DWORD PTR )?es:\[rdi\])
[ 	]*[a-f0-9]+:	a7 *	cmps(d *| +DWORD PTR (ds:)?\[rsi\],(DWORD PTR )?es:\[rdi\])
[ 	]*[a-f0-9]+:	48 a7 *	cmps(q *| +QWORD PTR (ds:)?\[rsi\],(QWORD PTR )?es:\[rdi\])
[ 	]*[a-f0-9]+:	48 a7 *	cmps(q *| +QWORD PTR (ds:)?\[rsi\],(QWORD PTR )?es:\[rdi\])
[ 	]*[a-f0-9]+:	64 48 a7 *	cmps +QWORD PTR fs:?\[rsi\],(QWORD PTR )?es:\[rdi\]
[ 	]*[a-f0-9]+:	48 a7 *	cmps(q *| +QWORD PTR (ds:)?\[rsi\],(QWORD PTR )?es:\[rdi\])
[ 	]*[a-f0-9]+:	48 a7 *	cmps(q *| +QWORD PTR (ds:)?\[rsi\],(QWORD PTR )?es:\[rdi\])
[ 	]*[a-f0-9]+:	48 a7 *	cmps(q *| +QWORD PTR (ds:)?\[rsi\],(QWORD PTR )?es:\[rdi\])
[ 	]*[a-f0-9]+:	48 a7 *	cmps(q *| +QWORD PTR (ds:)?\[rsi\],(QWORD PTR )?es:\[rdi\])
[ 	]*[a-f0-9]+:	48 a7 *	cmps(q *| +QWORD PTR (ds:)?\[rsi\],(QWORD PTR )?es:\[rdi\])
[ 	]*[a-f0-9]+:	48 a7 *	cmps(q *| +QWORD PTR (ds:)?\[rsi\],(QWORD PTR )?es:\[rdi\])
[ 	]*[a-f0-9]+:	48 a7 *	cmps(q *| +QWORD PTR (ds:)?\[rsi\],(QWORD PTR )?es:\[rdi\])
[ 	]*[a-f0-9]+:	67 48 a7 *	cmps +QWORD PTR (ds:)?\[esi\],(QWORD PTR )?es:\[edi\]
[ 	]*[a-f0-9]+:	64 67 48 a7 *	cmps +QWORD PTR fs:?\[esi\],(QWORD PTR )?es:\[edi\]
[ 	]*[a-f0-9]+:	67 48 a7 *	cmps +QWORD PTR (ds:)?\[esi\],(QWORD PTR )?es:\[edi\]
[ 	]*[a-f0-9]+:	67 48 a7 *	cmps +QWORD PTR (ds:)?\[esi\],(QWORD PTR )?es:\[edi\]
[ 	]*[a-f0-9]+:	67 48 a7 *	cmps +QWORD PTR (ds:)?\[esi\],(QWORD PTR )?es:\[edi\]
[ 	]*[a-f0-9]+:	67 48 a7 *	cmps +QWORD PTR (ds:)?\[esi\],(QWORD PTR )?es:\[edi\]
[ 	]*[a-f0-9]+:	67 48 a7 *	cmps +QWORD PTR (ds:)?\[esi\],(QWORD PTR )?es:\[edi\]
[ 	]*[a-f0-9]+:	67 48 a7 *	cmps +QWORD PTR (ds:)?\[esi\],(QWORD PTR )?es:\[edi\]
[ 	]*[a-f0-9]+:	67 48 a7 *	cmps +QWORD PTR (ds:)?\[esi\],(QWORD PTR )?es:\[edi\]
#pass
