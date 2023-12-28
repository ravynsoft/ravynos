#name: MIPS16 and microMIPS interlink
#source: ../../../gas/testsuite/gas/mips/nop.s -mips16
#source: ../../../gas/testsuite/gas/mips/nop.s -mmicromips
#ld: -e0
#error: \A.*: .*\.o: ASE mismatch: linking microMIPS module with previous MIPS16 modules[\n\r]+.*: failed to merge target specific data of file .*\.o\Z
