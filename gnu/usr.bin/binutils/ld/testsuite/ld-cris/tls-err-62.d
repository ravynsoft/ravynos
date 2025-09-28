#source: tls-err-62.s
#as: --pic --no-underscore --em=criself -I$srcdir/$subdir
#ld: --shared -m crislinux
#error: \A[^\n]*\.o[^\n]*relocation R_CRIS_32_GOT_TPREL with non-zero addend 42 against symbol `tls128'[^\n]*\n[^\n]*bad value\Z

# Check that non-zero addend on a R_CRIS_32_GOT_TPREL is flagged as an
# error, local symbol.
