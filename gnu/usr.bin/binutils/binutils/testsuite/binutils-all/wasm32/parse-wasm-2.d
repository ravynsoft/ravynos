#PROG: objcopy
#source: parse-wasm-2.s
#as:
#objcopy: -Ielf32-wasm32 -Obinary
#objdump: -bwasm -s

.*:.*file format wasm

Contents of section .wasm.type:
 80000000 01600001 7f                          .`...           
Contents of section .wasm.function:
 80000005 0100                                 ..              
Contents of section .wasm.code:
 80000007 01858080 80000041 2a0f0b             .......A\*..     
#pass
