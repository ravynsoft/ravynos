#as:
#source: diag-cttname-null.s
#objdump: --ctf
#ld: -shared --ctf-variables
#name: Diagnostics - Null type name

.*: +file format .*

Contents of CTF section .ctf:

  Header:
    Magic number: 0xdff2
    Version: 4 \(CTF_VERSION_3\)
#...
  Data objects:
    a -> 0x[0-9a-f]*: \(kind 6\) struct  \(size 0x[0-9a-f]*\) \(aligned at 0x[0-9a-f]*\)
#...
  Types:
#...
    0x[0-9a-f]*: \(kind 6\) struct  \(.*
        *\[0x0\] b: ID 0x[0-9a-f]*: \(kind 3\) struct B \* \(.*
#...
