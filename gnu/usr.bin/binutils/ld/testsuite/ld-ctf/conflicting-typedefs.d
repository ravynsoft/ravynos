#as:
#source: typedef-int.c
#source: typedef-long.c
#objdump: --ctf
#ld: -shared
#name: Conflicting Typedefs

.*: +file format .*

Contents of CTF section .ctf:

  Header:
    Magic number: 0xdff2
    Version: 4 \(CTF_VERSION_3\)
#...
  Types:
    0x1: .*int .*
    0x[0-9]: \(kind 10\) word .* -> 0x[0-9]: \(kind 1\) .*int \(format 0x1\) \(size 0x[0-9a-f]*\) \(aligned at 0x[0-9a-f]*\)
    0x[0-9]:.*int .*

  Strings:
#...
CTF archive member: .*typedef.*\.c:
#...
  Types:
    0x80000001: \(kind 10\) word .* -> 0x[0-9]: \(kind 1\) .*int \(format 0x1\) \(size 0x[0-9a-f]*\) \(aligned at 0x[0-9a-f]*\)

  Strings:
#...
