#PROG: objcopy
#source: invalid-wasm-2.s
#as:
#objcopy: -Ielf32-wasm32 -Obinary
#objdump: -bwasm -sD
#exit: 1