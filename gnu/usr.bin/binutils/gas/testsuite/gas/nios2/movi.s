# Source file used to test that former silent conversion of
# movi to orhi etc now gives range errors instead.

foo:
# This doesn't get converted.
movi r2, 0x20	

# This used to convert.
movi r2, 0x20000000

# addi used to convert only if the source register is r0.
addi r2, r0, 0xffff0000

# Logical ops used to convert to equivalent *hi for any register.
ori r2, r5, 0xffff0000
xori r2, r10, 0xffff0000
andi r2, r15, 0xffff0000

# This one used to be buggy and convert even though it wasn't supposed to,
# because it was failing to take the %lo relocation into account.
ori   r23,r23,%lo(0x12340000)
