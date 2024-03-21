#!/bin/sh
#
# Sudo Bug 361:
# Exercises a bug in the redblack tree code.
#

: ${VISUDO=visudo}

$VISUDO -cf - <<EOF
User_Alias	A=a
User_Alias	B=a
User_Alias	C=a
User_Alias	D=a
User_Alias	E=a
User_Alias	F=a
User_Alias	G=a
User_Alias	H=a
User_Alias	I=a
User_Alias	J=a
User_Alias	K=a
User_Alias	L=a
User_Alias	M=a

C	ALL=(ALL) ALL
E	ALL=(ALL) ALL
J	ALL=(ALL) ALL
D	ALL=(ALL) ALL
L	ALL=(ALL) ALL
H	ALL=(ALL) ALL
F	ALL=(ALL) ALL
G	ALL=(ALL) ALL
M	ALL=(ALL) ALL
K	ALL=(ALL) ALL
I	ALL=(ALL) ALL
EOF

exit 0
