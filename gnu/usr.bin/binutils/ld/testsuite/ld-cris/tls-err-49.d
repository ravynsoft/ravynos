#source: start1.s
#source: tls-ie-8.s --pic
#as: --no-underscore --em=criself
#ld: -m crislinux
#error: \A[^\n]*\.o: in function[^\n]*\n[^\n]*undefined reference[^\n]*\Z

# Undefined reference for a R_CRIS_16_GOT_TPREL in an executable.
