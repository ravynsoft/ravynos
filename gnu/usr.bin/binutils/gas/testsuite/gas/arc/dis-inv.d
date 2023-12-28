#objdump: -j .text -dr
#name: Check for correct disassembly of scondd llockd.
#as: -mcpu=archs

.*: +file format .*arc.*


Disassembly of section .text:

00000000 <.text>:
[\t ]+0\:[\t ]+262f 0052[\t ]+llockd[\t ]+r6\,\[r1\]
[\t ]+4\:[\t ]+262f 0053[\t ]+scondd[\t ]+r6\,\[r1\]
