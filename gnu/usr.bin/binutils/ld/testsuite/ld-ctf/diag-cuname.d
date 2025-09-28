#as:
#source: diag-cuname.s
#objdump: --ctf
#ld: -shared --ctf-variables
#name: Diagnostics - Invalid CU name offset

.*: +file format .*

Contents of CTF section .ctf:

  Header:
    Magic number: 0xdff2
    Version: 4 \(CTF_VERSION_3\)
#...
    Compilation unit name: \(\?\)
#...
    Data object section:	.*
    Type section:	.*
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
    0x[0-9a-f]*: \(\?\)
#...
