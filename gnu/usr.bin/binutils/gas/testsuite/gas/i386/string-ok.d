#as: -J
#objdump: -dw -mi386
#name: string insn operands
#warning_output: string-ok.e
.*: +file format .*

Disassembly of section .text:

0+ <.*start32>:
[ 	]+[0-9a-f]+:	2e a6[ 	]+cmpsb  (%es:)?\(%edi\),%cs:\(%esi\)
[ 	]+[0-9a-f]+:	a6[ 	]+cmpsb  (%es:)?\(%edi\),(%ds:)?\(%esi\)
[ 	]+[0-9a-f]+:	67 a6[ 	]+cmpsb  (%es:)?\(%di\),(%ds:)?\(%si\)
[ 	]+[0-9a-f]+:	a6[ 	]+cmpsb  (%es:)?\(%edi\),(%ds:)?\(%esi\)
[ 	]+[0-9a-f]+:	6c[ 	]+insb   \(%dx\),(%es:)?\(%edi\)
[ 	]+[0-9a-f]+:	6c[ 	]+insb   \(%dx\),(%es:)?\(%edi\)
[ 	]+[0-9a-f]+:	2e ac[ 	]+lods   %cs:\(%esi\),%al
[ 	]+[0-9a-f]+:	ac[ 	]+lods   (%ds:)?\(%esi\),%al
[ 	]+[0-9a-f]+:	2e a4[ 	]+movsb  %cs:\(%esi\),(%es:)?\(%edi\)
[ 	]+[0-9a-f]+:	a4[ 	]+movsb  (%ds:)?\(%esi\),(%es:)?\(%edi\)
[ 	]+[0-9a-f]+:	67 a4[ 	]+movsb  (%ds:)?\(%si\),(%es:)?\(%di\)
[ 	]+[0-9a-f]+:	a4[ 	]+movsb  (%ds:)?\(%esi\),(%es:)?\(%edi\)
[ 	]+[0-9a-f]+:	a4[ 	]+movsb  (%ds:)?\(%esi\),(%es:)?\(%edi\)
[ 	]+[0-9a-f]+:	2e 6e[ 	]+outsb  %cs:\(%esi\),\(%dx\)
[ 	]+[0-9a-f]+:	6e[ 	]+outsb  (%ds:)?\(%esi\),\(%dx\)
[ 	]+[0-9a-f]+:	ae[ 	]+scas   (%es:)?\(%edi\),%al
[ 	]+[0-9a-f]+:	ae[ 	]+scas   (%es:)?\(%edi\),%al
[ 	]+[0-9a-f]+:	aa[ 	]+stos   %al,(%es:)?\(%edi\)
[ 	]+[0-9a-f]+:	aa[ 	]+stos   %al,(%es:)?\(%edi\)
[ 	]+[0-9a-f]+:	d7[ 	]+xlatb? +(%ds:)?\(%ebx\)
[ 	]+[0-9a-f]+:	67 d7[ 	]+xlatb? +(%ds:)?\(%bx\)
[ 	]+[0-9a-f]+:	d7[ 	]+xlatb? +(%ds:)?\(%ebx\)
[ 	]+[0-9a-f]+:	d7[ 	]+xlatb? +(%ds:)?\(%ebx\)
[ 	]+[0-9a-f]+:	d7[ 	]+xlatb? +(%ds:)?\(%ebx\)
[ 	]+[0-9a-f]+:	2e d7[ 	]+xlatb? +%cs:\(%ebx\)

[0-9a-f]+ <.*start16>:
[ 	]+[0-9a-f]+:	a6[ 	]+cmpsb  (%es:)?\(%edi\),(%ds:)?\(%esi\)
[ 	]+[0-9a-f]+:	67 a4[ 	]+movsb  (%ds:)?\(%si\),(%es:)?\(%di\)

[0-9a-f]+ <.*start64>:
[ 	]+[0-9a-f]+:	a6[ 	]+cmpsb  (%es:)?\(%edi\),(%ds:)?\(%esi\)
[ 	]+[0-9a-f]+:	67 a4[ 	]+movsb  (%ds:)?\(%si\),(%es:)?\(%di\)

[0-9a-f]+ <.*intel32>:
[ 	]+[0-9a-f]+:	2e a6[ 	]+cmpsb  (%es:)?\(%edi\),%cs:\(%esi\)
[ 	]+[0-9a-f]+:	a6[ 	]+cmpsb  (%es:)?\(%edi\),(%ds:)?\(%esi\)
[ 	]+[0-9a-f]+:	a6[ 	]+cmpsb  (%es:)?\(%edi\),(%ds:)?\(%esi\)
[ 	]+[0-9a-f]+:	67 a6[ 	]+cmpsb  (%es:)?\(%di\),(%ds:)?\(%si\)
[ 	]+[0-9a-f]+:	a6[ 	]+cmpsb  (%es:)?\(%edi\),(%ds:)?\(%esi\)
[ 	]+[0-9a-f]+:	6c[ 	]+insb   \(%dx\),(%es:)?\(%edi\)
[ 	]+[0-9a-f]+:	6c[ 	]+insb   \(%dx\),(%es:)?\(%edi\)
[ 	]+[0-9a-f]+:	2e ac[ 	]+lods   %cs:\(%esi\),%al
[ 	]+[0-9a-f]+:	ac[ 	]+lods   (%ds:)?\(%esi\),%al
[ 	]+[0-9a-f]+:	2e a4[ 	]+movsb  %cs:\(%esi\),(%es:)?\(%edi\)
[ 	]+[0-9a-f]+:	a4[ 	]+movsb  (%ds:)?\(%esi\),(%es:)?\(%edi\)
[ 	]+[0-9a-f]+:	a4[ 	]+movsb  (%ds:)?\(%esi\),(%es:)?\(%edi\)
[ 	]+[0-9a-f]+:	67 a4[ 	]+movsb  (%ds:)?\(%si\),(%es:)?\(%di\)
[ 	]+[0-9a-f]+:	a4[ 	]+movsb  (%ds:)?\(%esi\),(%es:)?\(%edi\)
[ 	]+[0-9a-f]+:	a4[ 	]+movsb  (%ds:)?\(%esi\),(%es:)?\(%edi\)
[ 	]+[0-9a-f]+:	2e 6e[ 	]+outsb  %cs:\(%esi\),\(%dx\)
[ 	]+[0-9a-f]+:	6e[ 	]+outsb  (%ds:)?\(%esi\),\(%dx\)
[ 	]+[0-9a-f]+:	ae[ 	]+scas   (%es:)?\(%edi\),%al
[ 	]+[0-9a-f]+:	ae[ 	]+scas   (%es:)?\(%edi\),%al
[ 	]+[0-9a-f]+:	aa[ 	]+stos   %al,(%es:)?\(%edi\)
[ 	]+[0-9a-f]+:	aa[ 	]+stos   %al,(%es:)?\(%edi\)
[ 	]+[0-9a-f]+:	d7[ 	]+xlat   (%ds:)?\(%ebx\)
[ 	]+[0-9a-f]+:	67 d7[ 	]+xlat   (%ds:)?\(%bx\)
[ 	]+[0-9a-f]+:	d7[ 	]+xlat   (%ds:)?\(%ebx\)
[ 	]+[0-9a-f]+:	d7[ 	]+xlat   (%ds:)?\(%ebx\)
[ 	]+[0-9a-f]+:	2e d7[ 	]+xlat   %cs:\(%ebx\)

[0-9a-f]+ <.*intel16>:
[ 	]+[0-9a-f]+:	a6[ 	]+cmpsb  (%es:)?\(%edi\),(%ds:)?\(%esi\)
[ 	]+[0-9a-f]+:	67 a4[ 	]+movsb  (%ds:)?\(%si\),(%es:)?\(%di\)

[0-9a-f]+ <.*intel64>:
[ 	]+[0-9a-f]+:	a6[ 	]+cmpsb  (%es:)?\(%edi\),(%ds:)?\(%esi\)
[ 	]+[0-9a-f]+:	67 a4[ 	]+movsb  (%ds:)?\(%si\),(%es:)?\(%di\)
#pass
