#name: Missing SVE2 AES argument
#as: -march=armv8-a+sve2+sve2-sm4+sve2-sha3+sve2-bitperm
#source: sve2.s
#error: [^ :]+: Assembler messages:
#error: [^ :]+:[0-9]+: Error: selected processor does not support `aesd z17\.b,z17\.b,z21\.b'
#error: [^ :]+:[0-9]+: Error: selected processor does not support `aesd z0\.b,z0\.b,z0\.b'
#error: [^ :]+:[0-9]+: Error: selected processor does not support `aese z17\.b,z17\.b,z21\.b'
#error: [^ :]+:[0-9]+: Error: selected processor does not support `aese z0\.b,z0\.b,z0\.b'
#error: [^ :]+:[0-9]+: Error: selected processor does not support `aesimc z17\.b,z17\.b'
#error: [^ :]+:[0-9]+: Error: selected processor does not support `aesimc z0\.b,z0\.b'
#error: [^ :]+:[0-9]+: Error: selected processor does not support `aesmc z17\.b,z17\.b'
#error: [^ :]+:[0-9]+: Error: selected processor does not support `aesmc z0\.b,z0\.b'
#error: [^ :]+:[0-9]+: Error: selected processor does not support `pmullb z17\.q,z21\.d,z27\.d'
#error: [^ :]+:[0-9]+: Error: selected processor does not support `pmullb z0\.q,z0\.d,z0\.d'
#error: [^ :]+:[0-9]+: Error: selected processor does not support `pmullt z17\.q,z21\.d,z27\.d'
#error: [^ :]+:[0-9]+: Error: selected processor does not support `pmullt z0\.q,z0\.d,z0\.d'
