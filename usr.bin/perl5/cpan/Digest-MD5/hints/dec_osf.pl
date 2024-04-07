if ($] < 5.00503 and !$Config{gccversion}) {
  print "
  Because of a bug with the DEC system C compiler, some tests in
  t/rfc2202.t will be skipped.  These tests fail because the compiler
  bug breaks Perl's 'x' operator for eight-bit characters.  The
  Digest:: modules themselves work and should be safe to install
  anyway.

  Versions of Perl after 5.005_03 will contain a workaround for the
  bug.

";
}
