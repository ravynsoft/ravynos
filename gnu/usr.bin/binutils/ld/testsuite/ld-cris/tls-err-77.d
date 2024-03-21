#source: tls-ie-8e.s
#source: tls-x.s
#as: --no-underscore --em=criself
#ld: --shared -m crislinux
#error: \A[^\n]*\.o, [^\n]*\n[^\n]*mixup[^\n]*\n[^\n]*invalid operation\Z

# Check that a R_CRIS_32_IE in input to a DSO is flagged as an error.
