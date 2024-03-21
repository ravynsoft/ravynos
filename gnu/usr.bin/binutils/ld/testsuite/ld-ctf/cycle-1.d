#as:
#source: cycle-1.c
#source: A.c
#source: B.c
#source: C.c
#objdump: --ctf
#ld: -shared --ctf-variables
#name: Cycle 1

.*: +file format .*

Contents of CTF section .ctf:

  Header:
    Magic number: 0xdff2
    Version: 4 \(CTF_VERSION_3\)
#...
    Type section:	.* \(0xa8 bytes\)
    String section:	.*

  Labels:

  Data objects:

  Function objects:

  Variables:
#...
  Types:
#...
    0x[0-9a-f]*: \(kind 6\) struct cycle_1 \(.*
        *\[0x0\] a: ID 0x[0-9a-f]*: \(kind 3\) struct A \* \(.*
        *\[0x[0-9a-f]*\] b: ID 0x[0-9a-f]*: \(kind 3\) struct B \* \(.*
        *\[0x[0-9a-f]*\] next: ID 0x[0-9a-f]*: \(kind 3\) struct cycle_1 \* \(.*
#...
