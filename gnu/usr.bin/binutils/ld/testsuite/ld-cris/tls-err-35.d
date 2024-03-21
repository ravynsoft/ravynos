#source: start1.s
#source: tls-tprelm.s --defsym r=32768
#as: --no-underscore --em=criself
#ld: -m crislinux
#error: \A[^\n]*\.o: in function[^\n]*\n[^\n]*truncated[^\n]*\n[^\n]*too big[^\n]*\Z

# Check that overflow for R_CRIS_16_TPREL is flagged as an error.
