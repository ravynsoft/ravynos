#!./perl

# These tests are in a separate file, because they use fresh_perl_is()
# from test.pl.

# The mb* functions use the "underlying locale" that is not affected by
# the Perl one.  So we run the tests in a separate "fresh_perl" process
# with the correct LC_CTYPE set in the environment.

BEGIN {
    require Config; import Config;
    if ($^O ne 'VMS' and $Config{'extensions'} !~ /\bPOSIX\b/) {
	print "1..0\n";
	exit 0;
    }
    unshift @INC, "../../t";
    require 'loc_tools.pl';
    require 'charset_tools.pl';
    require 'test.pl';
}

my $utf8_locale = find_utf8_ctype_locale();

plan tests => 13;

use POSIX qw();

SKIP: {
    skip("mblen() not present", 7) unless $Config{d_mblen};

    is(&POSIX::mblen("a", &POSIX::MB_CUR_MAX), 1, 'mblen() works on ASCII input');
    is(&POSIX::mblen("b"), 1, '... and the 2nd parameter is optional');

    skip("LC_CTYPE locale support not available", 4)
      unless locales_enabled('LC_CTYPE');

    skip("no utf8 locale available", 4) unless $utf8_locale;
    # Here we need to influence LC_CTYPE, but it's not enough to just
    # set this because LC_ALL could override it. It's also not enough
    # to delete LC_ALL because it could be used to override other
    # variables such as LANG in the underlying test environment.
    # Continue to set LC_CTYPE just in case...
    local $ENV{LC_CTYPE} = $utf8_locale;
    local $ENV{LC_ALL} = $utf8_locale;

    fresh_perl_like(
        'use POSIX; print &POSIX::MB_CUR_MAX',
      qr/[4-6]/, {}, 'MB_CUR_MAX is at least 4 in a UTF-8 locale');

  SKIP: {
    my ($major, $minor, $rest) = $Config{osvers} =~ / (\d+) \. (\d+) .* /x;
    skip("mblen() broken (at least for c.utf8) on early HP-UX", 3)
        if   $Config{osname} eq 'hpux'
          && $major < 11 || ($major == 11 && $minor < 31);

    fresh_perl_is(
        'use POSIX; &POSIX::mblen(undef,0); print &POSIX::mblen("'
      . I8_to_native("\x{c3}\x{28}")
      . '", 2)',
      -1, {}, 'mblen() recognizes invalid multibyte characters');

    fresh_perl_is(
     'use POSIX; &POSIX::mblen(undef,0);
      my $sigma = "\N{GREEK SMALL LETTER SIGMA}";
      utf8::encode($sigma);
      print &POSIX::mblen($sigma, 2)',
     2, {}, 'mblen() works on UTF-8 characters');

    fresh_perl_is(
     'use POSIX; &POSIX::mblen(undef,0);
      my $wide; print &POSIX::mblen("\N{GREEK SMALL LETTER SIGMA}", 1);',
     -1, {}, 'mblen() returns -1 when input length is too short');
  }
}

SKIP: {
    skip("mbtowc() not present", 5) unless $Config{d_mbtowc} || $Config{d_mbrtowc};

    my $wide;

    is(&POSIX::mbtowc($wide, "a"), 1, 'mbtowc() returns correct length on ASCII input');
    is($wide , ord "a", 'mbtowc() returns correct ordinal on ASCII input');

    skip("LC_CTYPE locale support not available", 3)
      unless locales_enabled('LC_CTYPE');

    skip("no utf8 locale available", 3) unless $utf8_locale;

    local $ENV{LC_CTYPE} = $utf8_locale;
    local $ENV{LC_ALL} = $utf8_locale;
    local $ENV{PERL_UNICODE};
    delete $ENV{PERL_UNICODE};

  SKIP: {
    my ($major, $minor, $rest) = $Config{osvers} =~ / (\d+) \. (\d+) .* /x;
    skip("mbtowc() broken (at least for c.utf8) on early HP-UX", 3)
        if   $Config{osname} eq 'hpux'
          && $major < 11 || ($major == 11 && $minor < 31);

    fresh_perl_is(
        'use POSIX; &POSIX::mbtowc(undef, undef,0); my $wide; print &POSIX::mbtowc($wide, "'
      . I8_to_native("\x{c3}\x{28}")
      . '", 2)',
      -1, {}, 'mbtowc() recognizes invalid multibyte characters');

    fresh_perl_is(
     'use POSIX; &POSIX::mbtowc(undef,undef,0);
      my $sigma = "\N{GREEK SMALL LETTER SIGMA}";
      utf8::encode($sigma);
      my $wide; my $len = &POSIX::mbtowc($wide, $sigma, 2);
      print "$len:$wide"',
     "2:963", {}, 'mbtowc() works on UTF-8 characters');

    fresh_perl_is(
     'use POSIX; &POSIX::mbtowc(undef,undef,0);
      my $wide; print &POSIX::mbtowc($wide, "\N{GREEK SMALL LETTER SIGMA}", 1);',
     -1, {}, 'mbtowc() returns -1 when input length is too short');
  }
}

SKIP: {
    skip("wctomb() not present", 2) unless $Config{d_wctomb} || $Config{d_wcrtomb};

    fresh_perl_is('use POSIX; &POSIX::wctomb(undef,0); my $string; my $len = &POSIX::wctomb($string, ord "A"); print "$len:$string"',
      "1:A", {}, 'wctomb() works on ASCII input');

    skip("LC_CTYPE locale support not available", 1)
      unless locales_enabled('LC_CTYPE');

    skip("no utf8 locale available", 1) unless $utf8_locale;

    local $ENV{LC_CTYPE} = $utf8_locale;
    local $ENV{LC_ALL} = $utf8_locale;
    local $ENV{PERL_UNICODE};
    delete $ENV{PERL_UNICODE};

  SKIP: {
    my ($major, $minor, $rest) = $Config{osvers} =~ / (\d+) \. (\d+) .* /x;
    skip("wctomb() broken (at least for c.utf8) on early HP-UX", 1)
        if   $Config{osname} eq 'hpux'
          && $major < 11 || ($major == 11 && $minor < 31);

    fresh_perl_is('use POSIX; &POSIX::wctomb(undef,0); my $string; my $len = &POSIX::wctomb($string, 0x100); print "$len:$string"',
      "2:" . I8_to_native("\x{c4}\x{80}"),
      {}, 'wctomb() works on UTF-8 characters');

  }
}
