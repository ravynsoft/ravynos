#as:
#source: diag-parlabel.s
#objdump: --ctf
#ld: -shared --ctf-variables
#name: Diagnostics - Non-zero parlabel in parent

.*: +file format .*

Contents of CTF section .ctf:

  Header:
    Magic number: 0xdff2
    Version: 4 \(CTF_VERSION_3\)
#...
    Compilation unit name: .*A.c
    Data object section:	.* \(0x[1-9a-f][0-9a-f]* bytes\)
    Type section:	.* \(0x30 bytes\)
    String section:	.*

  Labels:

  Data objects:
    a -> 0x[0-9a-f]*: \(kind 6\) struct A \(size 0x[0-9a-f]*\) \(aligned at 0x[0-9a-f]*\)
#...
  Function objects:

  Variables:

  Types:
#...
    0x[0-9a-f]*: \(kind 6\) struct A \(.*
        *\[0x0\] b: ID 0x[0-9a-f]*: \(kind 3\) struct B \* \(.*
#...
  Strings:
    0x0: 
#...
    0x[0-9a-f]*: A
#...
