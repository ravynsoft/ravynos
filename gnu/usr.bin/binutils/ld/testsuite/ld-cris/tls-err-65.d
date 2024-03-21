#source: start1.s
#source: tls-le-12s.s
#as: --no-underscore --em=criself
#ld: -m crislinux tmpdir/tls-dso-xz-1.so
#error: \A[^\n]*\.o,[^\n]*mixup[^\n]*\n[^\n]*bad value\Z

# R_CRIS_16_TPREL in executable against symbol from DSO.
