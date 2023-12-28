#as:
#source: slice.c
#objdump: --ctf
#ld: -shared --ctf-variables
#name: Slice

.*: +file format .*

Contents of CTF section .ctf:

  Header:
    Magic number: 0xdff2
    Version: 4 \(CTF_VERSION_3\)
#...
    Compilation unit name: .*slice.c
#...
    Data object section:	.* \(0x[1-9a-f][0-9a-f]* bytes\)
    Type section:	.* \(0xe0 bytes\)
    String section:	.*
#...
  Data objects:
    slices -> 0x[0-9a-f]*: \(kind 6\) struct slices \(size 0x[0-9a-f]*\) \(aligned at 0x1*\)
#...
  Types:
#...
    0x[0-9a-f]*: \(kind 6\) struct slices \(size 0x[0-9a-f]*\) \(aligned at 0x[0-9a-f]*\)
        *\[0x0\] one: ID 0x[0-9a-f]*: \(kind 1\) int:1 \[slice 0x0:0x1\] \(format 0x1\) \(size 0x1\) \(aligned at 0x1\)
        *\[0x1\] two: ID 0x[0-9a-f]*: \(kind 1\) int:2 \[slice 0x0:0x2\] \(format 0x1\) \(size 0x1\) \(aligned at 0x1\)
        *\[0x3\] six: ID 0x[0-9a-f]*: \(kind 1\) int:6 \[slice 0x0:0x6\] \(format 0x1\) \(size 0x1\) \(aligned at 0x1\)
        *\[0x9\] ten: ID 0x[0-9a-f]*: \(kind 1\) int:10 \[slice 0x0:0xa\] \(format 0x1\) \(size 0x2\) \(aligned at 0x2\)
        *\[0x13\] bar: ID 0x[0-9a-f]*: \(kind 8\) enum foo:1 \[slice 0x0:0x1\] \(format 0x1\) \(size 0x1\) \(aligned at 0x1\)

#...
