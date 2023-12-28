#source: tls-ld-6.s --pic
#as: --no-underscore --em=criself
#ld: -m crislinux --shared
#error: \A[^\n]*\.o[^\n]*undefined reference[^\n]*\n[^\n]*bad value\Z

# Undefined reference for a R_CRIS_32_DTPREL in a DSO.
