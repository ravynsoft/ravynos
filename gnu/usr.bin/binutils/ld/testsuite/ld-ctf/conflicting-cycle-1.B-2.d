#as:
#source: cycle-1.c
#source: A.c
#source: B.c
#source: B-2.c
#source: C.c
#objdump: --ctf
#ld: -shared --ctf-variables
#name: Conflicting cycle 1.B-2

.*: +file format .*

#...
CTF archive member: .*/B-2.c:

  Header:
    Magic number: 0xdff2
    Version: 4 \(CTF_VERSION_3\)
#...
    Parent name: .ctf
    Compilation unit name: .*/B-2.c
#...
    Type section:	.* \(0x24 bytes\)
    String section:	.*

  Labels:

  Data objects:

  Function objects:

  Variables:
    b -> 0x80000001: \(kind 6\) struct B \(.*

  Types:
    0x8[0-9a-f]*: \(kind 6\) struct B \(.*
        *\[0x0\] c: ID 0x[0-9a-f]*: \(kind 3\) struct C \* \(.*
        *\[0x[0-9a-f]*\] wombat: ID 0x[0-9a-f]*: \(kind 1\) int \(format 0x1\) \(size 0x[0-9a-f]*\) \(aligned at 0x[0-9a-f]*\)

  Strings:
#...
