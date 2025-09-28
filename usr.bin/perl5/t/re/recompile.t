#!./perl

# Check that we don't recompile runtime patterns when the pattern hasn't
# changed
#
# Works by checking the debugging output of 'use re debug' and, if
# available, -Dr. We use both to check that the different code paths
# with Perl_foo() versus the my_foo() under ext/re/ don't cause any
# changes.

$| = 1;

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc( '../lib', '.' );
    skip_all_if_miniperl("no dynamic loading on miniperl, no re");
}

use strict;
use warnings;

plan tests => 48;

my $results = runperl(
			switches => [ '-Dr' ],
			prog => '1',
			stderr   => 1,
		    );
my $has_Dr = $results !~ /Recompile perl with -DDEBUGGING/;

my $tmpfile = tempfile();


# Check that a pattern triggers a regex compilation exactly N times,
# using either -Dr or 'use re debug'
# This is partially based on _fresh_perl() in test.pl

sub _comp_n {
    my ($use_Dr, $n, $prog, $desc) = @_;
    open my $tf, ">$tmpfile" or die "Cannot open $tmpfile: $!";

    my $switches = [];
    if ($use_Dr) {
	push @$switches, '-Dr';
    }
    else {
	$prog = qq{use re qw(debug);\n$prog};
    }

    print $tf $prog;
    close $tf or die "Cannot close $tmpfile: $!";
    my $results = runperl(
			switches => $switches,
			progfile => $tmpfile,
			stderr   => 1,
		    );

    my $status = $?;

    my $count = () = $results =~ /Final program:/g;
    if ($count == $n && !$status) {
	pass($desc);
    }
    else {
	fail($desc);
        _diag "# COUNT:    $count EXPECTED $n\n";
        _diag "# STATUS:   $status\n";
        _diag "# SWITCHES: @$switches\n";
        _diag "# PROG: \n$prog\n";
	# this is verbose; uncomment for debugging
        #_diag "# OUTPUT:\n------------------\n $results-------------------\n";
    }
}

# Check that a pattern triggers a regex compilation exactly N times,

sub comp_n {
    my ($n, $prog, $desc) = @_;
    if ($has_Dr) {
	_comp_n(1, $n, $prog, "$desc -Dr");
    }
    else {
	SKIP: {
	    skip("-Dr not compiled in");
	}
    }
    _comp_n(0, @_);
}

# Check that a pattern triggers a regex compilation exactly once.

sub comp_1 {
    comp_n(1, @_);
}


comp_1(<<'CODE', 'simple');
"a" =~ /$_/ for qw(a a a);
CODE

comp_1(<<'CODE', 'simple qr');
"a" =~ qr/$_/ for qw(a a a);
CODE

comp_1(<<'CODE', 'literal utf8');
"a" =~ /$_/ for "\x{100}", "\x{100}", "\x{100}";
CODE

comp_1(<<'CODE', 'literal utf8 qr');
"a" =~ qr/$_/ for "\x{100}", "\x{100}", "\x{100}";
CODE

comp_1(<<'CODE', 'longjmp literal utf8');
my $x = chr(0x80);
"a" =~ /$x$_/ for "\x{100}", "\x{100}", "\x{100}";
CODE

comp_1(<<'CODE', 'longjmp literal utf8 qr');
my $x = chr(0x80);
"a" =~ qr/$x$_/ for "\x{100}", "\x{100}", "\x{100}";
CODE

comp_1(<<'CODE', 'utf8');
"a" =~ /$_/ for '\x{100}', '\x{100}', '\x{100}';
CODE

comp_1(<<'CODE', 'utf8 qr');
"a" =~ qr/$_/ for '\x{100}', '\x{100}', '\x{100}';
CODE

comp_1(<<'CODE', 'longjmp utf8');
my $x = chr(0x80);
"a" =~ /$x$_/ for '\x{100}', '\x{100}', '\x{100}';
CODE

comp_1(<<'CODE', 'longjmp utf8');
my $x = chr(0x80);
"a" =~ qr/$x$_/ for '\x{100}', '\x{100}', '\x{100}';
CODE

comp_n(3, <<'CODE', 'mixed utf8');
"a" =~ /$_/ for "\x{c4}\x{80}",  "\x{100}", "\x{c4}\x{80}";
CODE

comp_n(3, <<'CODE', 'mixed utf8 qr');
"a" =~ qr/$_/ for "\x{c4}\x{80}",  "\x{100}", "\x{c4}\x{80}";
CODE

# note that for runtime code, each pattern is compiled twice; the
# second time to allow the parser to see the code.

comp_n(6, <<'CODE', 'runtime code');
my $x = '(?{1})';
BEGIN { $^H |= 0x00200000 } # lightweight "use re 'eval'"
"a" =~ /a$_/ for $x, $x, $x;
CODE

comp_n(6, <<'CODE', 'runtime code qr');
my $x = '(?{1})';
BEGIN { $^H |= 0x00200000 } # lightweight "use re 'eval'"
"a" =~ qr/a$_/ for $x, $x, $x;
CODE

comp_n(4, <<'CODE', 'embedded code');
my $x = qr/(?{1})/;
"a" =~ /a$_/ for $x, $x, $x;
CODE

comp_n(4, <<'CODE', 'embedded code qr');
my $x = qr/(?{1})/;
"a" =~ qr/a$_/ for $x, $x, $x;
CODE

comp_n(7, <<'CODE', 'mixed code');
my $x = qr/(?{1})/;
my $y = '(?{1})';
BEGIN { $^H |= 0x00200000 } # lightweight "use re 'eval'"
"a" =~ /a$x$_/ for $y, $y, $y;
CODE

comp_n(7, <<'CODE', 'mixed code qr');
my $x = qr/(?{1})/;
my $y = '(?{1})';
BEGIN { $^H |= 0x00200000 } # lightweight "use re 'eval'"
"a" =~ qr/a$x$_/ for $y, $y, $y;
CODE

comp_n(6, <<'CODE', 'embedded code qr');
my $x = qr/a/i;
my $y = qr/a/;
"a" =~ qr/a$_/ for $x, $y, $x, $y;
CODE

comp_n(2, <<'CODE', '(??{"constant"})');
"bb" =~ /(??{"abc"})/;
CODE

comp_n(2, <<'CODE', '(??{"folded"."constant"})');
"bb" =~ /(??{"ab"."c"})/;
CODE

comp_n(2, <<'CODE', '(??{$preused_scalar})');
$s = "abc";
"bb" =~ /(??{$s})/;
CODE

comp_n(2, <<'CODE', '(??{number})');
"bb" =~ /(??{123})/;
CODE

comp_n(2, <<'CODE', '(??{$pvlv_regexp})');
sub {
   $_[0] = ${qr/abc/};
  "bb" =~ /(??{$_[0]})/;
}->($_[0]);
CODE
