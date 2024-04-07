#!./perl

my $PERLIO;

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
    skip_all_without_perlio();
    # FIXME - more of these could be tested without Encode or full perl
    skip_all_without_dynamic_extension('Encode');

    # Makes testing easier.
    $ENV{PERLIO} = 'stdio' if exists $ENV{PERLIO} && $ENV{PERLIO} eq '';
    skip_all("PERLIO='$ENV{PERLIO}' unknown")
	if exists $ENV{PERLIO} && $ENV{PERLIO} !~ /^(stdio|perlio|mmap)$/;
    $PERLIO = exists $ENV{PERLIO} ? $ENV{PERLIO} : "(undef)";
}

use Config;

my $DOSISH    = $^O =~ /^(?:MSWin32|os2)$/ ? 1 : 0;
my $NONSTDIO  = exists $ENV{PERLIO} && $ENV{PERLIO} ne 'stdio'     ? 1 : 0;
my $FASTSTDIO = $Config{d_faststdio} && $Config{usefaststdio}      ? 1 : 0;
my $UTF8_STDIN;
if (${^UNICODE} & 1) {
    if (${^UNICODE} & 64) {
	# Conditional on the locale
	$UTF8_STDIN = ${^UTF8LOCALE};
    } else {
	# Unconditional
	$UTF8_STDIN = 1;
    }
} else {
    $UTF8_STDIN = 0;
}
my $NTEST = 62 - (($DOSISH || !$FASTSTDIO) ? 7 : 0) - ($DOSISH ? 7 : 0)
    + $UTF8_STDIN;

sub PerlIO::F_UTF8 () { 0x00008000 } # from perliol.h

plan tests => $NTEST;

print <<__EOH__;
# PERLIO        = $PERLIO
# DOSISH        = $DOSISH
# NONSTDIO      = $NONSTDIO
# FASTSTDIO     = $FASTSTDIO
# UNICODE       = ${^UNICODE}
# UTF8LOCALE    = ${^UTF8LOCALE}
# UTF8_STDIN = $UTF8_STDIN
__EOH__

{
    sub check {
	my ($result, $expected, $id) = @_;
	# An interesting dance follows where we try to make the following
	# IO layer stack setups to compare equal:
	#
	# PERLIO     UNIX-like                   DOS-like
	#
	# unset / "" unix perlio / stdio [1]     unix crlf
	# stdio      unix perlio / stdio [1]     stdio
	# perlio     unix perlio                 unix perlio
	# mmap       unix mmap                   unix mmap
	#
	# [1] "stdio" if Configure found out how to do "fast stdio" (depends
	# on the stdio implementation) and in Perl 5.8, otherwise "unix perlio"
	#
	if ($NONSTDIO) {
	    # Get rid of "unix".
	    shift @$result if $result->[0] eq "unix";
	    # Change expectations.
	    if ($FASTSTDIO) {
		$expected->[0] = $ENV{PERLIO};
	    } else {
		$expected->[0] = $ENV{PERLIO} if $expected->[0] eq "stdio";
	    }
	} elsif (!$FASTSTDIO && !$DOSISH) {
	    splice(@$result, 0, 2, "stdio")
		if @$result >= 2 &&
		   $result->[0] eq "unix" &&
		   $result->[1] eq "perlio";
	} elsif ($DOSISH) {
	    splice(@$result, 0, 2, "stdio")
		if @$result >= 2 &&
		   $result->[0] eq "unix" &&
		   $result->[1] eq "crlf";
	}
	if ($DOSISH && grep { $_ eq 'crlf' } @$expected) {
	    # 5 tests potentially skipped because
	    # DOSISH systems already have a CRLF layer
	    # which will make new ones not stick.
	    splice @$expected, 1, 1 if $expected->[1] eq 'crlf';
	}
	my $n = scalar @$expected;
	is(scalar @$result, $n, "$id - layers == $n");
	for (my $i = 0; $i < $n; $i++) {
	    my $j = $expected->[$i];
	    if (ref $j eq 'CODE') {
		ok($j->($result->[$i]), "$id - $i is ok");
	    } else {
		is($result->[$i], $j,
		   sprintf("$id - $i is %s",
			   defined $j ? $j : "undef"));
	    }
	}
    }

    check([ PerlIO::get_layers(STDIN) ],
	  $UTF8_STDIN ? [ "stdio", "utf8" ] : [ "stdio" ],
	  "STDIN");

    my $afile = tempfile();
    open(F, ">:crlf", $afile);

    check([ PerlIO::get_layers(F) ],
	  [ qw(stdio crlf) ],
	  "open :crlf");

    binmode(F, ":crlf");

    check([ PerlIO::get_layers(F) ],
	  [ qw(stdio crlf) ],
	  "binmode :crlf");

    binmode(F, ":encoding(cp1047)"); 

    check([ PerlIO::get_layers(F) ],
	  [ qw[stdio crlf encoding(cp1047) utf8] ],
	  ":encoding(cp1047)");

    binmode(F, ":crlf");

    check([ PerlIO::get_layers(F) ],
	  [ qw[stdio crlf encoding(cp1047) utf8 crlf utf8] ],
	  ":encoding(cp1047):crlf");
    
    binmode(F, ":pop:pop");

    check([ PerlIO::get_layers(F) ],
	  [ qw(stdio crlf) ],
	  ":pop");

    binmode(F, ":raw");

    check([ PerlIO::get_layers(F) ],
	  [ "stdio" ],
	  ":raw");

    binmode(F, ":utf8");

    check([ PerlIO::get_layers(F) ],
	  [ qw(stdio utf8) ],
	  ":utf8");

    binmode(F, ":bytes");

    check([ PerlIO::get_layers(F) ],
	  [ "stdio" ],
	  ":bytes");

    binmode(F, ":encoding(utf8)");

    check([ PerlIO::get_layers(F) ],
	    [ qw[stdio encoding(utf8) utf8] ],
	    ":encoding(utf8)");

    binmode(F, ":raw :crlf");

    check([ PerlIO::get_layers(F) ],
	  [ qw(stdio crlf) ],
	  ":raw:crlf");

    binmode(F, ":raw :encoding(latin1)"); # "latin1" will be canonized

    # 7 tests potentially skipped.
    unless ($DOSISH || !$FASTSTDIO) {
	my @results = PerlIO::get_layers(F, details => 1);

	# Get rid of the args and the flags.
	splice(@results, 1, 2) if $NONSTDIO;

	check([ @results ],
	      [ "stdio",    undef,        sub { $_[0] > 0 },
		"encoding", "iso-8859-1", sub { $_[0] & PerlIO::F_UTF8() } ],
	      ":raw:encoding(latin1)");
    }

    binmode(F);

    check([ PerlIO::get_layers(F) ],
	  [ "stdio" ],
	  "binmode");

    check([ PerlIO::get_layers(*F{IO}) ],
	  [ "stdio" ],
	  "binmode");

    # RT78844
    {
        local $@ = "foo";
        binmode(F, ":encoding(utf8)");
        is( $@, "foo", '$@ not clobbered by binmode and :encoding');
    }

    close F;

    {
	use open(IN => ":crlf", OUT => ":encoding(cp1252)");

	open F, '<', $afile;
	open G, '>', $afile;

	check([ PerlIO::get_layers(F, input  => 1) ],
	      [ qw(stdio crlf) ],
	      "use open IN");
	
	check([ PerlIO::get_layers(G, output => 1) ],
	      [ qw[stdio encoding(cp1252) utf8] ],
	      "use open OUT");

	close F;
	close G;
    }

    # Check that PL_sigwarn's reference count is correct, and that 
    # &PerlIO::Layer::NoWarnings isn't prematurely freed.
    fresh_perl_like (<<"EOT", qr/^CODE/, {}, "Check PL_sigwarn's reference count");
open(UTF, "<:raw:encoding(utf8)", '$afile') or die \$!;
print ref *PerlIO::Layer::NoWarnings{CODE};
EOT

    # [perl #97956] Not calling FETCH all the time on tied variables
    my $f;
    sub TIESCALAR { bless [] }
    sub FETCH { ++$f; $_[0][0] = $_[1] }
    sub STORE { $_[0][0] }
    tie my $t, "";
    $t = *f;
    $f = 0; PerlIO::get_layers $t;
    is $f, 1, '1 fetch on tied glob';
    $t = \*f;
    $f = 0; PerlIO::get_layers $t;
    is $f, 1, '1 fetch on tied globref';
    $t = *f;
    $f = 0; PerlIO::get_layers \$t;
    is $f, 1, '1 fetch on referenced tied glob';
    $t = '';
    $f = 0; PerlIO::get_layers $t;
    is $f, 1, '1 fetch on tied string';

    # No distinction between nums and strings
    open "12", "<:crlf", "test.pl" or die "$0 cannot open test.pl: $!";
    ok PerlIO::get_layers(12), 'str/num arguments are treated identically';
}
