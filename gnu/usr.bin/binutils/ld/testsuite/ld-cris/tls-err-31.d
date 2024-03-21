#source: tls-dtprelm.s --defsym r=32768
#as: --no-underscore --em=criself --pic
#ld: --shared -m crislinux
#error: \A[^\n]*\.o: in function[^\n]*\n[^\n]*truncated[^\n]*\n[^\n]*too big[^\n]*\Z

# Check that overflow for R_CRIS_16_DTPREL is flagged as an error.
