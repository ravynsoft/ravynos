#as:
#source: cycle-1.c
#source: A.c
#source: A-2.c
#source: B.c
#source: B-2.c
#source: C.c
#source: C-2.c
#objdump: --ctf
#ld: -shared --ctf-variables
#name: Conflicting cycle 2.parent

.*: +file format .*

Contents of CTF section .ctf:

  Header:
    Magic number: 0xdff2
    Version: 4 \(CTF_VERSION_3\)
#...
    Type section:	.* \(0x94 bytes\)
    String section:	.* \(0x1d bytes\)

  Labels:

  Data objects:

  Function objects:

  Variables:
    cycle_1 -> 0x[0-9a-f]*: \(kind 3\) struct cycle_1 \* \(size 0x[0-9a-f]*\) \(aligned at 0x[0-9a-f]*\) -> 0x[0-9a-f]*: \(kind 6\) struct cycle_1 \(size 0x[0-9a-f]*\) \(aligned at 0x[0-9a-f]*\)

  Types:
#...
    0x[0-9a-f]*: \(kind 6\) struct cycle_1 \(size 0x[0-9a-f]*\) \(aligned at 0x[0-9a-f]*\)
        *\[0x0\] a: ID 0x[0-9a-f]*: \(kind 3\) struct A \* \(size 0x[0-9a-f]*\) \(aligned at 0x[0-9a-f]*\)
        *\[0x[0-9a-f]*\] b: ID 0x[0-9a-f]*: \(kind 3\) struct B \* \(size 0x[0-9a-f]*\) \(aligned at 0x[0-9a-f]*\)
        *\[0x[0-9a-f]*\] next: ID 0x[0-9a-f]*: \(kind 3\) struct cycle_1 \* \(size 0x[0-9a-f]*\) \(aligned at 0x[0-9a-f]*\)
#...
