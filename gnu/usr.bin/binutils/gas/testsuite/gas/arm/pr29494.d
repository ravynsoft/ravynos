# name: PR 29494: Trailing jump table => unaligned opcode
# objdump: -d
# Assembling the pr29494.s source file used to generate a 
#  "unaligned opcodes detected in executable segment"
# message because the jump table at the end of the .text
# section was not 2-byte aligned.

#...
.*\.short[ 	]+0x(aea8|a8ae)
.*\.short[ 	]+0x(b3c1|c1b3)
.*\.byte[ 	]+0xb8
#pass
