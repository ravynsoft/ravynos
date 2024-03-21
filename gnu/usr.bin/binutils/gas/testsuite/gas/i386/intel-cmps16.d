#as: --defsym x86_16=1
#objdump: -dMintel -Mi8086
#source: intel-cmps.s
#name: x86 Intel cmps (16-bit code)

.*: +file format .*

Disassembly of section .text:

0+ <cmps>:
[ 	]*[a-f0-9]+:	a6 *	cmps(b *| +BYTE PTR (ds:)?\[si\],(BYTE PTR )?es:\[di\])
[ 	]*[a-f0-9]+:	67 a6 *	cmps +BYTE PTR (ds:)?\[esi\],(BYTE PTR )?es:\[edi\]
[ 	]*[a-f0-9]+:	64 67 a6 *	cmps +BYTE PTR fs:\[esi\],(BYTE PTR )?es:\[edi\]
[ 	]*[a-f0-9]+:	67 a6 *	cmps +BYTE PTR (ds:)?\[esi\],(BYTE PTR )?es:\[edi\]
[ 	]*[a-f0-9]+:	67 a6 *	cmps +BYTE PTR (ds:)?\[esi\],(BYTE PTR )?es:\[edi\]
[ 	]*[a-f0-9]+:	67 a6 *	cmps +BYTE PTR (ds:)?\[esi\],(BYTE PTR )?es:\[edi\]
[ 	]*[a-f0-9]+:	67 a6 *	cmps +BYTE PTR (ds:)?\[esi\],(BYTE PTR )?es:\[edi\]
[ 	]*[a-f0-9]+:	67 a6 *	cmps +BYTE PTR (ds:)?\[esi\],(BYTE PTR )?es:\[edi\]
[ 	]*[a-f0-9]+:	67 a6 *	cmps +BYTE PTR (ds:)?\[esi\],(BYTE PTR )?es:\[edi\]
[ 	]*[a-f0-9]+:	67 a6 *	cmps +BYTE PTR (ds:)?\[esi\],(BYTE PTR )?es:\[edi\]
[ 	]*[a-f0-9]+:	a6 *	cmps(b *| +BYTE PTR (ds:)?\[si\],(BYTE PTR )?es:\[di\])
[ 	]*[a-f0-9]+:	64 a6 *	cmps +BYTE PTR fs:\[si\],(BYTE PTR )?es:\[di\]
[ 	]*[a-f0-9]+:	a6 *	cmps(b *| +BYTE PTR (ds:)?\[si\],(BYTE PTR )?es:\[di\])
[ 	]*[a-f0-9]+:	a6 *	cmps(b *| +BYTE PTR (ds:)?\[si\],(BYTE PTR )?es:\[di\])
[ 	]*[a-f0-9]+:	a6 *	cmps(b *| +BYTE PTR (ds:)?\[si\],(BYTE PTR )?es:\[di\])
[ 	]*[a-f0-9]+:	a6 *	cmps(b *| +BYTE PTR (ds:)?\[si\],(BYTE PTR )?es:\[di\])
[ 	]*[a-f0-9]+:	a6 *	cmps(b *| +BYTE PTR (ds:)?\[si\],(BYTE PTR )?es:\[di\])
[ 	]*[a-f0-9]+:	a6 *	cmps(b *| +BYTE PTR (ds:)?\[si\],(BYTE PTR )?es:\[di\])
[ 	]*[a-f0-9]+:	a6 *	cmps(b *| +BYTE PTR (ds:)?\[si\],(BYTE PTR )?es:\[di\])
[ 	]*[a-f0-9]+:	a7 *	cmps(w *| +WORD PTR (ds:)?\[si\],(WORD PTR )?es:\[di\])
[ 	]*[a-f0-9]+:	67 a7 *	cmps +WORD PTR (ds:)?\[esi\],(WORD PTR )?es:\[edi\]
[ 	]*[a-f0-9]+:	64 67 a7 *	cmps +WORD PTR fs:\[esi\],(WORD PTR )?es:\[edi\]
[ 	]*[a-f0-9]+:	67 a7 *	cmps +WORD PTR (ds:)?\[esi\],(WORD PTR )?es:\[edi\]
[ 	]*[a-f0-9]+:	67 a7 *	cmps +WORD PTR (ds:)?\[esi\],(WORD PTR )?es:\[edi\]
[ 	]*[a-f0-9]+:	67 a7 *	cmps +WORD PTR (ds:)?\[esi\],(WORD PTR )?es:\[edi\]
[ 	]*[a-f0-9]+:	67 a7 *	cmps +WORD PTR (ds:)?\[esi\],(WORD PTR )?es:\[edi\]
[ 	]*[a-f0-9]+:	67 a7 *	cmps +WORD PTR (ds:)?\[esi\],(WORD PTR )?es:\[edi\]
[ 	]*[a-f0-9]+:	67 a7 *	cmps +WORD PTR (ds:)?\[esi\],(WORD PTR )?es:\[edi\]
[ 	]*[a-f0-9]+:	67 a7 *	cmps +WORD PTR (ds:)?\[esi\],(WORD PTR )?es:\[edi\]
[ 	]*[a-f0-9]+:	a7 *	cmps(w *| +WORD PTR (ds:)?\[si\],(WORD PTR )?es:\[di\])
[ 	]*[a-f0-9]+:	64 a7 *	cmps +WORD PTR fs:\[si\],(WORD PTR )?es:\[di\]
[ 	]*[a-f0-9]+:	a7 *	cmps(w *| +WORD PTR (ds:)?\[si\],(WORD PTR )?es:\[di\])
[ 	]*[a-f0-9]+:	a7 *	cmps(w *| +WORD PTR (ds:)?\[si\],(WORD PTR )?es:\[di\])
[ 	]*[a-f0-9]+:	a7 *	cmps(w *| +WORD PTR (ds:)?\[si\],(WORD PTR )?es:\[di\])
[ 	]*[a-f0-9]+:	a7 *	cmps(w *| +WORD PTR (ds:)?\[si\],(WORD PTR )?es:\[di\])
[ 	]*[a-f0-9]+:	a7 *	cmps(w *| +WORD PTR (ds:)?\[si\],(WORD PTR )?es:\[di\])
[ 	]*[a-f0-9]+:	a7 *	cmps(w *| +WORD PTR (ds:)?\[si\],(WORD PTR )?es:\[di\])
[ 	]*[a-f0-9]+:	a7 *	cmps(w *| +WORD PTR (ds:)?\[si\],(WORD PTR )?es:\[di\])
[ 	]*[a-f0-9]+:	66 a7 *	cmps(d *| +DWORD PTR (ds:)?\[si\],(DWORD PTR )?es:\[di\])
[ 	]*[a-f0-9]+:	67 66 a7 *	cmps +DWORD PTR (ds:)?\[esi\],(DWORD PTR )?es:\[edi\]
[ 	]*[a-f0-9]+:	64 67 66 a7 *	cmps +DWORD PTR fs:?\[esi\],(DWORD PTR )?es:\[edi\]
[ 	]*[a-f0-9]+:	67 66 a7 *	cmps +DWORD PTR (ds:)?\[esi\],(DWORD PTR )?es:\[edi\]
[ 	]*[a-f0-9]+:	67 66 a7 *	cmps +DWORD PTR (ds:)?\[esi\],(DWORD PTR )?es:\[edi\]
[ 	]*[a-f0-9]+:	67 66 a7 *	cmps +DWORD PTR (ds:)?\[esi\],(DWORD PTR )?es:\[edi\]
[ 	]*[a-f0-9]+:	67 66 a7 *	cmps +DWORD PTR (ds:)?\[esi\],(DWORD PTR )?es:\[edi\]
[ 	]*[a-f0-9]+:	67 66 a7 *	cmps +DWORD PTR (ds:)?\[esi\],(DWORD PTR )?es:\[edi\]
[ 	]*[a-f0-9]+:	67 66 a7 *	cmps +DWORD PTR (ds:)?\[esi\],(DWORD PTR )?es:\[edi\]
[ 	]*[a-f0-9]+:	67 66 a7 *	cmps +DWORD PTR (ds:)?\[esi\],(DWORD PTR )?es:\[edi\]
[ 	]*[a-f0-9]+:	66 a7 *	cmps(d *| +DWORD PTR (ds:)?\[si\],(DWORD PTR )?es:\[di\])
[ 	]*[a-f0-9]+:	64 66 a7 *	cmps +DWORD PTR fs:?\[si\],(DWORD PTR )?es:\[di\]
[ 	]*[a-f0-9]+:	66 a7 *	cmps(d *| +DWORD PTR (ds:)?\[si\],(DWORD PTR )?es:\[di\])
[ 	]*[a-f0-9]+:	66 a7 *	cmps(d *| +DWORD PTR (ds:)?\[si\],(DWORD PTR )?es:\[di\])
[ 	]*[a-f0-9]+:	66 a7 *	cmps(d *| +DWORD PTR (ds:)?\[si\],(DWORD PTR )?es:\[di\])
[ 	]*[a-f0-9]+:	66 a7 *	cmps(d *| +DWORD PTR (ds:)?\[si\],(DWORD PTR )?es:\[di\])
[ 	]*[a-f0-9]+:	66 a7 *	cmps(d *| +DWORD PTR (ds:)?\[si\],(DWORD PTR )?es:\[di\])
[ 	]*[a-f0-9]+:	66 a7 *	cmps(d *| +DWORD PTR (ds:)?\[si\],(DWORD PTR )?es:\[di\])
[ 	]*[a-f0-9]+:	66 a7 *	cmps(d *| +DWORD PTR (ds:)?\[si\],(DWORD PTR )?es:\[di\])
#pass
