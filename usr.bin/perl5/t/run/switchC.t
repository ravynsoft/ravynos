#!./perl -w

# Tests for the command-line switches

BEGIN {
    chdir 't' if -d 't';
    @INC = '../lib';
    require "./test.pl";

    skip_all_without_perlio();
    skip_all_if_miniperl('-C and $ENV{PERL_UNICODE} are disabled on miniperl');
}

plan(tests => 15);

my $r;

my $tmpfile = tempfile();
my $scriptfile = tempfile();

my $b = chr 256; utf8::encode $b;

$r = runperl( switches => [ '-CO', '-w' ],
	      prog     => 'print chr(256)',
              stderr   => 1 );
like( $r, qr/^$b(?:\r?\n)?$/s, '-CO: no warning on UTF-8 output' );

$r = runperl( switches => [ '-C2', '-w' ],
	      prog     => 'print chr(256)',
              stderr   => 1 );
like( $r, qr/^$b(?:\r?\n)?$/s, '-C2: no warning on UTF-8 output' );

SKIP: {
    if (exists $ENV{PERL_UNICODE} &&
	($ENV{PERL_UNICODE} eq "" || $ENV{PERL_UNICODE} =~ /[SO]/)) {
	skip(qq[cannot test with PERL_UNICODE "" or /[SO]/], 1);
    }
    $r = runperl( switches => [ '-CI', '-w' ],
		  prog     => 'print ord(<STDIN>)',
		  stderr   => 1,
		  stdin    => $b );
    like( $r, qr/^256(?:\r?\n)?$/s, '-CI: read in UTF-8 input' );
}

$r = runperl( switches => [ '-CE', '-w' ],
	      prog     => 'warn chr(256), qq(\n)',
              stderr   => 1 );
like( $r, qr/^$b(?:\r?\n)?$/s, '-CE: UTF-8 stderr' );

$r = runperl( switches => [ '-Co', '-w' ],
	      prog     => "open(F, q(>$tmpfile)); print F chr(256); close F",
              stderr   => 1 );
like( $r, qr/^$/s, '-Co: auto-UTF-8 open for output' );

$r = runperl( switches => [ '-Ci', '-w' ],
	      prog     => "open(F, q(<$tmpfile)); print ord(<F>); close F",
              stderr   => 1 );
like( $r, qr/^256(?:\r?\n)?$/s, '-Ci: auto-UTF-8 open for input' );

open(S, ">$scriptfile") or die("open $scriptfile: $!");
print S "open(F, q(<$tmpfile)); print ord(<F>); close F";
close S;

$r = runperl( switches => [ '-Ci', '-w' ],
	      progfile => $scriptfile,
              stderr   => 1 );
like( $r, qr/^256(?:\r?\n)?$/s, '-Ci: auto-UTF-8 open for input affects the current file' );

$r = runperl( switches => [ '-Ci', '-w' ],
	      prog     => "do q($scriptfile)",
              stderr   => 1 );
unlike( $r, qr/^256(?:\r?\n)?$/s, '-Ci: auto-UTF-8 open for input has file scope' );

$r = runperl( switches => [ '-CA', '-w' ],
	      prog     => 'print ord shift',
              stderr   => 1,
              args     => [ chr(256) ] );
like( $r, qr/^256(?:\r?\n)?$/s, '-CA: @ARGV' );

$r = runperl( switches => [ '-CS', '-w' ],
	      progs    => [ '#!perl -CS', 'print chr(256)'],
              stderr   => 1, );
like( $r, qr/^$b(?:\r?\n)?$/s, '#!perl -C' );

$r = runperl( switches => [ '-CS' ],
	      progs    => [ '#!perl -CS -w', 'print chr(256), !!$^W'],
              stderr   => 1, );
like( $r, qr/^${b}1(?:\r?\n)?$/s, '#!perl -C followed by another switch' );

$r = runperl( switches => [ '-CS' ],
	      progs    => [ '#!perl -C7 -w', 'print chr(256), !!$^W'],
              stderr   => 1, );
like(
  $r, qr/^${b}1(?:\r?\n)?$/s,
 '#!perl -C<num> followed by another switch'
);

$r = runperl( switches => [ '-CA', '-w' ],
	      progs    => [ '#!perl -CS', 'print chr(256)' ],
              stderr   => 1, );
like( $r, qr/^Too late for "-CS" option at -e line 1\.$/s,
      '#!perl -C with different -C on command line' );

SKIP: {
    if (exists $ENV{PERL_UNICODE} && $ENV{PERL_UNICODE} =~ /S/) {
	skip(qq[cannot test with PERL_UNICODE including "S"], 1);
    }
    $r = runperl( switches => [ '-w' ],
                  progs    => [ '#!perl -CS', 'print chr(256)' ],
                  stderr   => 1, );
    like( $r, qr/^Too late for "-CS" option at -e line 1\.$/s,
          '#!perl -C but not command line' );
}

$r = runperl ( switches => [ '-C00' ],
               prog    => '1',
               stderr   => 1, );
like($r, qr/^Invalid number '00' for -C option\.$/s,
     "perl -C00 [perl #123991]");
