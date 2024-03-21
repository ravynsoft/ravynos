#objdump: -drw
#name: x86-64 w64 pcrel
#source: x86-64-pcrel.s

.*: +file format .*

Disassembly of section .text:

0+000 <_start>:
[	 ]*[0-9a-f]+:[	 ]+b0 02[	 ]+movb?[	 ]+\$(0x)?2,%al[	 ]*[0-9a-f]+:[	 ]+R_X86_64_PC8[	 ]+xtrn
[	 ]*[0-9a-f]+:[	 ]+66 b8 04 00[	 ]+movw?[	 ]+\$(0x)?4,%ax[	 ]*[0-9a-f]+:[	 ]+R_X86_64_PC16[	 ]+xtrn
[	 ]*[0-9a-f]+:[	 ]+b8 05( 00){3}[	 ]+movl?[	 ]+\$(0x)?5,%eax[	 ]*[0-9a-f]+:[	 ]+IMAGE_REL_AMD64_REL32[	 ]+xtrn
[	 ]*[0-9a-f]+:[	 ]+48 c7 c0 07( 00){3}[	 ]+movq?[	 ]+\$(0x)?7,%rax[	 ]*[0-9a-f]+:[	 ]+IMAGE_REL_AMD64_REL32[	 ]+xtrn
[	 ]*[0-9a-f]+:[	 ]+48 b8 0a( 00){7}[	 ]+mov(abs)?q?[	 ]+\$(0x)?a,%rax[	 ]*[0-9a-f]+:[	 ]+R_X86_64_PC64[	 ]+xtrn
[	 ]*[0-9a-f]+:[	 ]+b0 00[	 ]+movb?[	 ]+\$(0x)?0,%al[	 ]*[0-9a-f]+:[	 ]+R_X86_64_8[	 ]+xtrn
[	 ]*[0-9a-f]+:[	 ]+66 b8 00 00[	 ]+movw?[	 ]+\$(0x)?0,%ax[	 ]*[0-9a-f]+:[	 ]+R_X86_64_16[	 ]+xtrn
[	 ]*[0-9a-f]+:[	 ]+b8( 00){4}[	 ]+movl?[	 ]+\$(0x)?0,%eax[	 ]*[0-9a-f]+:[	 ]+IMAGE_REL_AMD64_ADDR32[	 ]+xtrn
[	 ]*[0-9a-f]+:[	 ]+48 c7 c0( 00){4}[	 ]+movq?[	 ]+\$(0x)?0,%rax[	 ]*[0-9a-f]+:[	 ]+R_X86_64_32S[	 ]+xtrn
[	 ]*[0-9a-f]+:[	 ]+48 b8( 00){8}[	 ]+mov(abs)?q?[	 ]+\$(0x)?0,%rax[	 ]*[0-9a-f]+:[	 ]+IMAGE_REL_AMD64_ADDR64[	 ]+xtrn
#pass
