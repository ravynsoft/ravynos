.*: Assembler messages:
.*:4: Error: .*
.*:5: Error: .*
.*:6: Error: .*
.*:9: Error: .*
.*:10: Error: .*
.*:11: Error: .*
GAS LISTING .*


[ 	]*1[ 	]+\# Check illegal 64bit AVX instructions
[ 	]*2[ 	]+\.text
[ 	]*3[ 	]+_start:
[ 	]*4[ 	]+vcvtpd2dq \(%rcx\),%xmm2
[ 	]*5[ 	]+vcvtpd2ps \(%rcx\),%xmm2
[ 	]*6[ 	]+vcvttpd2dq \(%rcx\),%xmm2
[ 	]*7[ 	]+
[ 	]*8[ 	]+\.intel_syntax noprefix
[ 	]*9[ 	]+vcvtpd2dq xmm2,\[rcx\]
[ 	]*10[ 	]+vcvtpd2ps xmm2,\[rcx\]
[ 	]*11[ 	]+vcvttpd2dq xmm2,\[rcx\]
