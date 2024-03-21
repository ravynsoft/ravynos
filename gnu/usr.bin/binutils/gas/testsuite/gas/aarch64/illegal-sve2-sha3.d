#name: Missing SVE2 SHA3 argument
#as: -march=armv8-a+sve2+sve2-sm4+sve2-aes+sve2-bitperm
#source: sve2.s
#error: [^ :]+: Assembler messages:
#error: [^ :]+:[0-9]+: Error: selected processor does not support `rax1 z17\.d,z21\.d,z27\.d'
#error: [^ :]+:[0-9]+: Error: selected processor does not support `rax1 z0\.d,z0\.d,z0\.d'
