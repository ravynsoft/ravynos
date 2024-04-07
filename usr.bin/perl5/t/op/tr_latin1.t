# Tests for tr, but the test file is not utf8.

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    skip_all('ASCII sensitive') if $::IS_EBCDIC;
    set_up_inc('../lib');
}

plan tests => 2;

{ # This test is malloc sensitive.  Right now on some platforms anyway, space
  # for the final \xff needs to be mallocd, and that's what caused the
  # problem, because the '-' had already been parsed and was later added
  # without making space for it
    fresh_perl_is('print "\x8c" =~ y o\x{100}ÄŒÿÿ€€-ÿoo', "1", { },
                    'RT #134067 heap-buffer-overflow in S_scan_const');

}

{   # gh#17277.  This caused errors with valgrind and asan
    fresh_perl_is('no warnings qw(void uninitialized); s~~00~-y~Ë0~\x{E00}~',
                  "", {}, 'gh#17227');
}

1;
