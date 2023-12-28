# This should match the output of gas -al cond-2.s.
.*: Assembler messages:
.*:1005: Warning: line 5
.*cond-2.s.*


[ 	]*[1-9][0-9]*[ 	]+\.if[ 	]+0[ 	]*
[ 	]*[1-9][0-9]*[ 	]+# 1003 "cond-2\.s"
[ 	]*[1-9][0-9]*[ 	]+\.endif[ 	]*
[ 	]*[1-9][0-9]*[ 	]*
[ 	]*[1-9][0-9]*[ 	]+\.warning[ 	].*
#pass
