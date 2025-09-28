#as:
#source: enum-forward.c
#objdump: --ctf
#ld: -shared
#name: Forwards to enums

.*: +file format .*

Contents of CTF section .ctf:

  Header:
    Magic number: 0xdff2
    Version: 4 \(CTF_VERSION_3\)
#...
    Type section:	.* \(0x48 bytes\)
#...
  Data objects:
    get_color_name -> 0x[0-9a-f]*: \(kind 3\) char \*\*\(\*\) \(enum vibgyor\) \(size 0x[0-9a-f]*\) \(aligned at 0x[0-9a-f]*\) -> 0x[0-9a-f]*: \(kind 5\) char \*\(\*\) \(enum vibgyor\) \(aligned at 0x[0-9a-f]*\)

#...
