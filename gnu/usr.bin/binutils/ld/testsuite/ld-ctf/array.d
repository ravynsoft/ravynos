#as:
#source: array-char.c
#source: array-int.c
#objdump: --ctf
#ld: -shared --ctf-variables --hash-style=sysv
#name: Arrays

.*: +file format .*

Contents of CTF section .ctf:

  Header:
    Magic number: 0xdff2
    Version: 4 \(CTF_VERSION_3\)
#...
    Data object section:	.* \(0x[1-9a-f][0-9a-f]* bytes\)
    Type section:	.* \(0x6c bytes\)
    String section:	.*

  Labels:

  Data objects:
    digits -> 0x[0-9a-f]*: \(kind 4\) int \[10\] .*
    digits_names -> 0x[0-9a-f]*: \(kind 4\) char \*\[10\] .*

  Function objects:

  Variables:

  Types:
#...
    0x[0-9a-f]*: \(kind 4\) .*\[10\] \(size .*
#...
    0x[0-9a-f]*: \(kind 4\) .*\[10\] \(size .*
#...
