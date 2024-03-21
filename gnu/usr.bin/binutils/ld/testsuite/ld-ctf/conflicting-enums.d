#as:
#source: enum.c
#source: enum-2.c
#objdump: --ctf
#ld: -shared
#name: Conflicting Enums

.*: +file format .*

Contents of CTF section .ctf:

  Header:
    Magic number: 0xdff2
    Version: 4 \(CTF_VERSION_3\)
#...
  Types:
#...
  Strings:
#...
CTF archive member: .*enum.*\.c:
#...
  Types:
    0x80000001: \(kind 8\) enum day_of_the_week \(size 0x[0-9a-f]*\) \(aligned at 0x[0-9a-f]*\)
                Sunday: 0
                Monday: 1
                Tuesday: 2
                Wednesday: 3
                Thursday: 4
                Friday: 5
                Saturday: 6

  Strings:
#...
CTF archive member: .*enum.*\.c:
#...
  Types:
    0x80000001: \(kind 8\) enum day_of_the_week \(size 0x[0-9a-f]*\) \(aligned at 0x[0-9a-f]*\)
                Monday: 0
                Tuesday: 1
                Wednesday: 2
                Thursday: 3
                Friday: 4
                Saturday: 5
                Sunday: 6

  Strings:
#...
