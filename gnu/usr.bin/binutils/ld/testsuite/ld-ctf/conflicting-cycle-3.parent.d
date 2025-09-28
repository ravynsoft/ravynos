#as:
#source: A.c
#source: A-2.c
#source: B.c
#source: B-2.c
#source: C.c
#source: C-2.c
#objdump: --ctf
#ld: -shared
#name: Conflicting cycle 3

.*: +file format .*

Contents of CTF section .ctf:

  Header:
    Magic number: 0xdff2
    Version: 4 \(CTF_VERSION_3\)
#...
    Type section:	0x0 -- 0x57 \(0x58 bytes\)
    String section:	.*

  Labels:

  Data objects:

  Function objects:

  Variables:

  Types:
#...
    0x[0-9a-f]*: \(kind 1\) int \(format 0x1\) \(size 0x[0-9a-f]*\) \(aligned at 0x[0-9a-f]*\)
#...
  Strings:
    0x0: 
#...
