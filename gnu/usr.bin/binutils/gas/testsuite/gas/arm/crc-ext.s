.text
.arch	armv8-a
.arch_extension	crc
crc32w r0,r0,r2

.fpu	crypto-neon-fp-armv8
crc32w r0,r0,r2

.arch	armv8.1-a
crc32w r0,r0,r2

.cpu cortex-a53
crc32w r0,r0,r2

.cpu cortex-a55
crc32w r0,r0,r2

.cpu cortex-a57
crc32w r0,r0,r2
