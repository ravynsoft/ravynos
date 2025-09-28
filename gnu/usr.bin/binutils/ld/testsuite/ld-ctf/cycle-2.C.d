#as:
#source: A.c
#source: B.c
#source: C.c
#objdump: --ctf
#ld: -shared --ctf-variables
#name: Cycle 2.C

.*: +file format .*

Contents of CTF section .ctf:

  Header:
    Magic number: 0xdff2
    Version: 4 \(CTF_VERSION_3\)
#...
    Type section:	.* \(0x6c bytes\)
    String section:	.*

  Labels:

  Data objects:

  Function objects:

  Variables:
#...
    c -> 0x[0-9a-f]*: \(kind 6\) struct C \(.*
#...
  Types:
#...
    0x[0-9a-f]*: \(kind 6\) struct C \(.*
        *\[0x0\] a: ID 0x[0-9a-f]*: \(kind 3\) struct A \* \(.*
#...
  Strings:
    0x0: 
#...
    0x[0-9a-f]*: C
#...
