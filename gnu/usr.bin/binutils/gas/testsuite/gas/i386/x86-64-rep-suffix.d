#objdump: -dwMsuffix
#name: x86-64 rep prefix (with suffixes)

.*: +file format .*

Disassembly of section .text:

0+000 <_start>:
   0:	f3 ac[ 	]+rep lodsb %ds:\(%rsi\),%al
   2:	f3 aa[ 	]+rep stosb %al,%es:\(%rdi\)
   4:	66 f3 ad[ 	]+rep lodsw %ds:\(%rsi\),%ax
   7:	66 f3 ab[ 	]+rep stosw %ax,%es:\(%rdi\)
   a:	f3 ad[ 	]+rep lodsl %ds:\(%rsi\),%eax
   c:	f3 ab[ 	]+rep stosl %eax,%es:\(%rdi\)
   e:	f3 48 ad[ 	]+rep lodsq %ds:\(%rsi\),%rax
  11:	f3 48 ab[ 	]+rep stosq %rax,%es:\(%rdi\)
  14:	f3 0f bc c1[	 ]+tzcntl %ecx,%eax
  18:	f3 0f bd c1[	 ]+lzcntl %ecx,%eax
  1c:	f3 c3[	 ]+repz retq\s*
  1e:	f3 90[	 ]+pause\s*
#pass
