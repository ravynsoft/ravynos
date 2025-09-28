#as:
#source: cycle-1.c
#source: A.c
#source: B.c
#source: B-2.c
#source: C.c
#objdump: --ctf
#ld: -shared --ctf-variables
#name: Conflicting cycle 1.B-1

.*: +file format .*

#...
CTF archive member: .*/B.c:

  Header:
    Magic number: 0xdff2
    Version: 4 \(CTF_VERSION_3\)
#...
    Parent name: .ctf
    Compilation unit name: .*/B.c
#...
    Type section:	.* \(0x18 bytes\)
    String section:	.*

  Labels:

  Data objects:

  Function objects:

  Variables:
    b -> 0x80000001: \(kind 6\) struct B \(size 0x[0-9a-f]*\) \(aligned at 0x[0-9a-f]*\)

  Types:
    0x8[0-9a-f]*: \(kind 6\) struct B .*
        *\[0x0\] c: ID 0x[0-9a-f]*: \(kind 3\) struct C \* \(.*

  Strings:
#...
