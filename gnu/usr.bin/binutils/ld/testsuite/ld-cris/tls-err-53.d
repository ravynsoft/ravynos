#source: start1.s
#source: tls-ld-4.s
#as: --no-underscore --em=criself --pic
#ld: -m crislinux tmpdir/tls-dso-xz-1.so
#error: \A[^\n]*\.o,[^\n]*mixup[^\n]*\n[^\n]*bad value\Z

# R_CRIS_16_DTPREL in executable against symbol from DSO.
