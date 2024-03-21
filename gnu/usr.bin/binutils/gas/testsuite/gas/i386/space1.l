.*: Assembler messages:
.*:2: Error: .space, .nops or .fill specifies non-absolute value
.*:3: Error: .space, .nops or .fill specifies non-absolute value
.*:4: Error: .space, .nops or .fill specifies non-absolute value
.*:5: Error: .space, .nops or .fill specifies non-absolute value
.*:6: Error: .space, .nops or .fill specifies non-absolute value
GAS LISTING .*


[ 	]*1[ 	]+\.text
[ 	]*2[ 	]+\.space foo
[ 	]*3[ 	]+\.space foo\*4
[ 	]*4[ 	]+\.space foo\+4
[ 	]*5[ 	]+\.space foo-4
[ 	]*6[ 	]+\.space foo << 4
