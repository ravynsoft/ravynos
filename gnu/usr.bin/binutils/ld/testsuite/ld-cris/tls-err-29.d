#source: init.s
#source: tls-gdgotrelm.s --defsym r=8192
#as: --no-underscore --em=criself --pic
#ld: --shared -m crislinux
#error: \A[^\n]*\.o: in function[^\n]*\n[^\n]*truncated[^\n]*\n[^\n]*too many[^\n]*\Z

# Check that overflow for R_CRIS_16_GOT_GD is flagged as an error.
