#PROG: objcopy
#source: parse-wasm.s
#as:
#objcopy: -Ielf32-wasm32 -Obinary
#objdump: -bwasm -s

.*:.*file format wasm

