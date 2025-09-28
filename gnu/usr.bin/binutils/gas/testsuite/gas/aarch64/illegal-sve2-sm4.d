#name: Missing SVE2 SM4 argument
#as: -march=armv8-a+sve2+sve2-sha3+sve2-aes+sve2-bitperm
#source: sve2.s
#error: [^ :]+: Assembler messages:
#error: [^ :]+:[0-9]+: Error: selected processor does not support `sm4e z17\.s,z17\.s,z21\.s'
#error: [^ :]+:[0-9]+: Error: selected processor does not support `sm4e z0\.s,z0\.s,z0\.s'
#error: [^ :]+:[0-9]+: Error: selected processor does not support `sm4ekey z17\.s,z21\.s,z27\.s'
#error: [^ :]+:[0-9]+: Error: selected processor does not support `sm4ekey z0\.s,z0\.s,z0\.s'
