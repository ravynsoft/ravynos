.*: Assembler messages:
.*:6: Error: .* mismatch for `div'
.*:7: Error: .* mismatch for `in'
.*:8: Error: .* mismatch for `lods'
.*:9: Error: .* mismatch for `movabs'
.*:10: Error: .* mismatch for `shl'
GAS LISTING .*


[ 	]*[1-9][0-9]*[ 	]+\# Check %axl / %cxl aren't permitted as accumulator / shift count
[ 	]*[1-9][0-9]*[ 	]+
[ 	]*[1-9][0-9]*[ 	]+\.text
[ 	]*[1-9][0-9]*[ 	]+reg:
[ 	]*[1-9][0-9]*[ 	]+\?* 4080C001[ 	]+add	\$1, %axl
[ 	]*[1-9][0-9]*[ 	]+div	%bl, %axl
[ 	]*[1-9][0-9]*[ 	]+in	%dx, %axl
[ 	]*[1-9][0-9]*[ 	]+lods	\(%rsi\), %axl
[ 	]*[1-9][0-9]*[ 	]+movabs	-1, %axl
[ 	]*[1-9][0-9]*[ 	]+shl	%cxl, %eax
[ 	]*[1-9][0-9]*[ 	]+\?* 40F6C001[ 	]+test	\$1, %axl
#pass
