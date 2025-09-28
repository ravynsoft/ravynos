#as:
#source: forward.c
#objdump: --ctf
#ld: -shared
#name: Forwards

.*: +file format .*

Contents of CTF section .ctf:

  Header:
    Magic number: 0xdff2
    Version: 4 \(CTF_VERSION_3\)
#...
    Type section:	.* \(0x18 bytes\)
#...
  Types:

    0x[0-9a-f]: \(kind 9\) struct foo
    0x[0-9a-f]: \(kind 3\) struct foo \* \(size 0x[0-9a-f]*\) \(aligned at 0x[0-9a-f]*\) -> 0x[0-9a-f]: \(kind 9\) struct foo
#...
