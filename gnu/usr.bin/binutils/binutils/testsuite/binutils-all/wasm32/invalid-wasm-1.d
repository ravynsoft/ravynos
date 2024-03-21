#PROG: objcopy
#source: invalid-wasm-1.s
#as:
#objcopy: -Ielf32-wasm32 -Obinary
#objdump: -bwasm -sD
#error: : File format not recognized