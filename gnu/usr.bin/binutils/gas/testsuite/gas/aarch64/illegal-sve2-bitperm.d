#name: Missing SVE2 BITPERM argument
#as: -march=armv8-a+sve2+sve2-sm4+sve2-sha3+sve2-aes
#source: sve2.s
#error: [^ :]+: Assembler messages:
#error: [^ :]+:[0-9]+: Error: selected processor does not support `bdep z17\.b,z21\.b,z27\.b'
#error: [^ :]+:[0-9]+: Error: selected processor does not support `bdep z0\.b,z0\.b,z0\.b'
#error: [^ :]+:[0-9]+: Error: selected processor does not support `bdep z0\.h,z0\.h,z0\.h'
#error: [^ :]+:[0-9]+: Error: selected processor does not support `bdep z0\.s,z0\.s,z0\.s'
#error: [^ :]+:[0-9]+: Error: selected processor does not support `bdep z0\.d,z0\.d,z0\.d'
#error: [^ :]+:[0-9]+: Error: selected processor does not support `bext z17\.b,z21\.b,z27\.b'
#error: [^ :]+:[0-9]+: Error: selected processor does not support `bext z0\.b,z0\.b,z0\.b'
#error: [^ :]+:[0-9]+: Error: selected processor does not support `bext z0\.h,z0\.h,z0\.h'
#error: [^ :]+:[0-9]+: Error: selected processor does not support `bext z0\.s,z0\.s,z0\.s'
#error: [^ :]+:[0-9]+: Error: selected processor does not support `bext z0\.d,z0\.d,z0\.d'
#error: [^ :]+:[0-9]+: Error: selected processor does not support `bgrp z17\.b,z21\.b,z27\.b'
#error: [^ :]+:[0-9]+: Error: selected processor does not support `bgrp z0\.b,z0\.b,z0\.b'
#error: [^ :]+:[0-9]+: Error: selected processor does not support `bgrp z0\.h,z0\.h,z0\.h'
#error: [^ :]+:[0-9]+: Error: selected processor does not support `bgrp z0\.s,z0\.s,z0\.s'
#error: [^ :]+:[0-9]+: Error: selected processor does not support `bgrp z0\.d,z0\.d,z0\.d'
