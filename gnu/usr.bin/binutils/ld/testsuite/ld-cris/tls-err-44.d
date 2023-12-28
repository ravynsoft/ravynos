#source: tls-le-12.s
#as: --no-underscore --em=criself
#ld: -m crislinux --shared
#error: \A[^\n]*\.o, [^\n]*\n[^\n]*mixup[^\n]*\n[^\n]*invalid operation\Z

# Undefined reference for a R_CRIS_32_TPREL in a DSO (where it's
# invalid in the first place anyway, so we should see the same
# behavior as for that test, tls-err-25.d).
