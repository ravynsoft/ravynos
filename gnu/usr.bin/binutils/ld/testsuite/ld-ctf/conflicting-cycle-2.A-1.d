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
#name: Conflicting cycle 2.A-1

.*: +file format .*

#...
CTF archive member: .*/A.c:

  Header:
    Magic number: 0xdff2
    Version: 4 \(CTF_VERSION_3\)
#...
    Parent name: .*
    Compilation unit name: .*/A.c
#...
  Labels:

  Data objects:

  Function objects:

  Variables:
    a -> 0x80000001: \(kind 6\) struct A \(size 0x[0-9a-f]*\) \(aligned at 0x[0-9a-f]*\)

  Types:
    0x8[0-9a-f]*: \(kind 6\) struct A \(size 0x[0-9a-f]*\) \(aligned at 0x[0-9a-f]*\)
        *\[0x0\] b: ID 0x[0-9a-f]*: \(kind 3\) struct B \* \(size 0x[0-9a-f]*\) \(aligned at 0x[0-9a-f]*\)

  Strings:
    0x0: 
#...
