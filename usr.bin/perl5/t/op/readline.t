#!./perl

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}

plan tests => 36;

# [perl #19566]: sv_gets writes directly to its argument via
# TARG. Test that we respect SvREADONLY.
use constant roref => \2;
eval { for (roref) { $_ = <FH> } };
like($@, qr/Modification of a read-only value attempted/, '[perl #19566]');

# [perl #21628]
{
  my $file = tempfile();
  open A,'+>',$file; $a = 3;
  is($a .= <A>, 3, '#21628 - $a .= <A> , A eof');
  close A; $a = 4;
  is($a .= <A>, 4, '#21628 - $a .= <A> , A closed');
}

# [perl #21614]: 82 is chosen to exceed the length for sv_grow in
# do_readline (80)
foreach my $k (1, 82) {
  my $result
    = runperl (stdin => '', stderr => 1,
              prog => "\$x = q(k) x $k; \$a{\$x} = qw(v); \$_ = <> foreach keys %a; print qw(end)",
	      );
  $result =~ s/\n\z// if $^O eq 'VMS';
  is ($result, "end", '[perl #21614] for length ' . length('k' x $k));
}


foreach my $k (1, 21) {
  my $result
    = runperl (stdin => ' rules', stderr => 1,
              prog => "\$x = q(perl) x $k; \$a{\$x} = q(v); foreach (keys %a) {\$_ .= <>; print}",
	      );
  $result =~ s/\n\z// if $^O eq 'VMS';
  is ($result, ('perl' x $k) . " rules", 'rcatline to shared sv for length ' . length('perl' x $k));
}

foreach my $l (1, 82) {
  my $k = $l;
  $k = 'k' x $k;
  my $copy = $k;
  $k = <DATA>;
  is ($k, "moo\n", 'catline to COW sv for length ' . length $copy);
}


foreach my $l (1, 21) {
  my $k = $l;
  $k = 'perl' x $k;
  my $perl = $k;
  $k .= <DATA>;
  is ($k, "$perl rules\n", 'rcatline to COW sv for length ' . length $perl);
}

use strict;

open F, '.' and binmode F and sysread F, $_, 1;
my $err = $! + 0;
close F;

SKIP: {
  skip "you can read directories as plain files", 2 unless( $err );

  $!=0;
  open F, '.' and $_=<F>;
  ok( $!==$err && !defined($_) => 'readline( DIRECTORY )' );
  close F;

  $!=0;
  { local $/;
    open F, '.' and $_=<F>;
    ok( $!==$err && !defined($_) => 'readline( DIRECTORY ) slurp mode' );
    close F;
  }
}

fresh_perl_is('BEGIN{<>}', '',
              { switches => ['-w'], stdin => '', stderr => 1 },
              'No ARGVOUT used only once warning');

fresh_perl_is('print readline', 'foo',
              { switches => ['-w'], stdin => 'foo', stderr => 1 },
              'readline() defaults to *ARGV');

# [perl #72720] Test that sv_gets clears any variables that should be
# empty so if the read() aborts with EINTER, the TARG is actually
# cleared.
sub test_eintr_readline {
    my ( $fh, $timeout ) = @_;

    # This variable, the TARG for the readline is the core of this
    # test. The test is to see that after a my() and a failure in
    # readline() has the variable revived old, "dead" values from the
    # past or is it still undef like expected.
    my $line;

    # Do a readline into $line.
    if ( $timeout ) {

	# Do a SIGALARM aborted readline(). The underlying sv_gets()
	# from sv.c will use the syscall read() while will exit early
	# and return something like EINTR or ERESTARTSYS.
	my $timed_out;
	my $errno;
	eval {
	    local $SIG{ALRM} = sub {
		$timed_out = 1;
		die 'abort this timeout';
	    };
	    alarm $timeout;
	    undef $!;
	    $line = readline $fh;
	    $errno = $!;
	    alarm 0;
	};

	# The code should have timed out.
	if ( ! $timed_out ) {
	    warn $@
                ? "$@: $errno\n"
                : "Interrupted readline() test couldn't get interrupted: $errno";
	}
    }
    else {
	$line = readline $fh;
    }
    return $line;
}
SKIP: {

    # Connect two handles together.
    my ( $in, $out );
    my $piped;
    eval {
	pipe $in, $out;
	$piped = 1;
    };
    if ( ! $piped ) {
	skip( 2, 'The pipe function is unimplemented' );
    }

    binmode $out;
    binmode $in;

    # Make the pipe autoflushing
    {
	my $old_fh = select $out;
	$| = 1;
	select $old_fh;
    }

    # Only one line is loaded into the pipe. It's written unbuffered
    # so I'm confident it'll not be buffered.
    syswrite $out, "once\n";

    # Buggy perls will return the last thing successfully
    # returned. Buggy perls will return "once\n" a second (and
    # "infinitely" if we desired) as long as the internal read()
    # syscall fails. In our case, it fails because the inner my($line)
    # retains all its allocated space and buggy perl sets SvPOK to
    # make the value valid but before it starts read().
    my $once  = test_eintr_readline( $in, 0 );
    is(   $once,  "once\n", "readline read first line ok" );

    my $twice;
    TODO: {
        todo_skip( 'alarm() on Windows does not interrupt system calls' ) if $^O eq 'MSWin32';
        todo_skip( 'readline not interrupted by alarm on VMS -- why?' ) if $^O eq 'VMS';
        $twice = test_eintr_readline( $in, 1 );
        isnt( $twice, "once\n", "readline didn't re-return things when interrupted" );
    }

    TODO: {
        todo_skip( 'alarm() on Windows does not interrupt system calls' ) if $^O eq 'MSWin32';
        todo_skip( 'readline not interrupted by alarm on VMS -- why?' ) if $^O eq 'VMS';
        local our $TODO = "bad readline returns '', not undef";
        is( $twice, undef, "readline returned undef when interrupted" );
    }
}

{
    my $line = 'ascii';
    my ( $in, $out );
    pipe $in, $out;
    binmode $in;
    binmode $out;
    syswrite $out, "...\n";
    $line .= readline $in;

    is( $line, "ascii...\n", 'Appending from ascii to ascii' );
}

{
    my $line = "\x{2080} utf8";
    my ( $in, $out );
    pipe $in, $out;
    binmode $out;
    binmode $in;
    syswrite $out, "...\n";
    $line .= readline $in;

    is( $line, "\x{2080} utf8...\n", 'Appending from ascii to utf8' );
}

{
    my $line = 'ascii';
    my ( $in, $out );
    pipe $in, $out;
    binmode $out;
    binmode $in,  ':utf8';
    syswrite $out, "...\n";
    $line .= readline $in;

    is( $line, "ascii...\n", 'Appending from utf8 to ascii' );
}

{
    my $line = "\x{2080} utf8";;
    my ( $in, $out );
    pipe $in, $out;
    binmode $out;
    binmode $in,  ':utf8';
    my $outdata = "\x{2080}...\n";
    utf8::encode($outdata);
    syswrite $out, $outdata;
    $line .= readline $in;

    is( $line, "\x{2080} utf8\x{2080}...\n", 'appending from utf to utf8' );
}

my $obj = bless [];
$obj .= <DATA>;
like($obj, qr/main=ARRAY.*world/, 'rcatline and refs');

# bug #38631
require Tie::Scalar;
tie our $one, 'Tie::StdScalar', "A: ";
tie our $two, 'Tie::StdScalar', "B: ";
my $junk = $one;
$one .= <DATA>;
$two .= <DATA>;
is( $one, "A: One\n", "rcatline works with tied scalars" );
is( $two, "B: Two\n", "rcatline works with tied scalars" );

# mentioned in bug #97482
# <$foo> versus readline($foo) should not affect vivification.
my $yunk = "brumbo";
if (exists $::{$yunk}) {
     die "Name $yunk already used. Please adjust this test."
}
<$yunk>;
ok !defined *$yunk, '<> does not autovivify';
readline($yunk);
ok !defined *$yunk, "readline does not autovivify";

# [perl #97988] PL_last_in_gv could end up pointing to junk.
#               Now glob copies set PL_last_in_gv to null when unglobbed.
open *foom,'test.pl';
my %f;
$f{g} = *foom;
readline $f{g};
$f{g} = 3; # PL_last_in_gv should be cleared now
is tell, -1, 'tell returns -1 after last gv is unglobbed';
$f{g} = *foom; # since PL_last_in_gv is null, this should have no effect
is tell, -1, 'unglobbery of last gv nullifies PL_last_in_gv';
readline *{$f{g}};
is tell, tell *foom, 'readline *$glob_copy sets PL_last_in_gv';

# PL_last_in_gv should not point to &PL_sv_undef, either.
# This used to fail an assertion or return a scalar ref.
readline undef;
is ${^LAST_FH}, undef, '${^LAST_FH} after readline undef';

{
    my $w;
    local($SIG{__WARN__},$^W) = (sub { $w .= shift }, 1);
    *x=<y>;
    like $w, qr/^readline\(\) on unopened filehandle y at .*\n(?x:
                )Undefined value assigned to typeglob at .*\n\z/,
        '[perl #123790] *x=<y> used to fail an assertion';
}

SKIP:
{
    skip_without_dynamic_extension("IO", 4);
    my $tmpfile = tempfile();
    open my $fh, ">", $tmpfile
        or die "Cannot open $tmpfile: $!";
    my @layers = PerlIO::get_layers($fh);
    skip "fgetc doesn't set error flag on failure on solaris likes", 4
        if $^O eq 'solaris' && $layers[-1] eq 'stdio';
    ok(!$fh->error, "no error before we try to read");
    ok(!<$fh>, "fail to readline file opened for write");
    ok($fh->error, "error after trying to readline file opened for write");
    ok(!close($fh), "closing the file should fail");
}

__DATA__
moo
moo
 rules
 rules
world
One
Two
