#source: tls-le-12s.s
#source: tls-z.s
#as: --no-underscore --em=criself
#ld: --shared -m crislinux
#error: \A[^\n]*\.o, [^\n]*\n[^\n]*mixup[^\n]*\n[^\n]*invalid operation\Z

# Check that R_CRIS_16_TPREL in input to a DSO is flagged as an error.
