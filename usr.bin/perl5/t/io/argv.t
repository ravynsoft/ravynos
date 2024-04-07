#!./perl

BEGIN {
    chdir 't' if -d 't';
    require "./test.pl";
    set_up_inc('../lib');
}

plan(tests => 37);

my ($devnull, $no_devnull);

if (is_miniperl()) {
    $no_devnull = "no dynamic loading on miniperl, File::Spec not built, so can't determine /dev/null";
} else {
    require File::Spec;
    $devnull = File::Spec->devnull;
}

open(TRY, '>tmpIo_argv1.tmp') || (die "Can't open temp file: $!");
print TRY "a line\n";
close TRY or die "Could not close: $!";
open(TRY, '>tmpIo_argv2.tmp') || (die "Can't open temp file: $!");
print TRY "another line\n";
close TRY or die "Could not close: $!";

$x = runperl(
    prog	=> 'while (<>) { print $., $_; }',
    args	=> [ 'tmpIo_argv1.tmp', 'tmpIo_argv1.tmp' ],
);
is($x, "1a line\n2a line\n", '<> from two files');

{
    $x = runperl(
	prog	=> 'while (<>) { print $_; }',
	stdin	=> "foo\n",
	args	=> [ 'tmpIo_argv1.tmp', '-' ],
    );
    is($x, "a line\nfoo\n", '<> from a file and STDIN');

    # readline should behave as <>, not <<>>
    $x = runperl(
        prog	=> 'while (readline) { print $_; }',
        stdin	=> "foo\n",
        stderr 	=> 1,
        args	=> [ '-' ],
    );
    is($x, "foo\n", 'readline() from STDIN');

    $x = runperl(
	prog	=> 'while (<>) { print $_; }',
	stdin	=> "foo\n",
    );
    is($x, "foo\n", '<> from just STDIN');

    $x = runperl(
	prog	=> 'while (<>) { print $ARGV.q/,/.$_ }',
	args	=> [ 'tmpIo_argv1.tmp', 'tmpIo_argv2.tmp' ],
    );
    is($x, "tmpIo_argv1.tmp,a line\ntmpIo_argv2.tmp,another line\n", '$ARGV is the file name');

TODO: {
        local $::TODO = "unrelated bug in redirection implementation" if $^O eq 'VMS';
        $x = runperl(
            prog	=> 'print $ARGV while <>',
            stdin	=> "foo\nbar\n",
            args   	=> [ '-' ],
        );
        is($x, "--", '$ARGV is - for explicit STDIN');

        $x = runperl(
            prog	=> 'print $ARGV while <>',
            stdin	=> "foo\nbar\n",
        );
        is($x, "--", '$ARGV is - for implicit STDIN');
    }
}

{
    # 5.10 stopped autovivifying scalars in globs leading to a
    # segfault when $ARGV is written to.
    runperl( prog => 'eof()', stdin => "nothing\n" );
    is( 0+$?, 0, q(eof() doesn't segfault) );
}

@ARGV = is_miniperl() ? ('tmpIo_argv1.tmp', 'tmpIo_argv1.tmp', 'tmpIo_argv1.tmp')
    : ('tmpIo_argv1.tmp', 'tmpIo_argv1.tmp', $devnull, 'tmpIo_argv1.tmp');
while (<>) {
    $y .= $. . $_;
    if (eof()) {
	is($., 3, '$. counts <>');
    }
}

is($y, "1a line\n2a line\n3a line\n", '<> from @ARGV');


open(TRY, '>tmpIo_argv1.tmp') or die "Can't open temp file: $!";
close TRY or die "Could not close: $!";
open(TRY, '>tmpIo_argv2.tmp') or die "Can't open temp file: $!";
close TRY or die "Could not close: $!";
@ARGV = ('tmpIo_argv1.tmp', 'tmpIo_argv2.tmp');
$^I = '_bak';   # not .bak which confuses VMS
$/ = undef;
my $i = 11;
while (<>) {
    s/^/ok $i\n/;
    ++$i;
    print;
    next_test();
}
open(TRY, '<tmpIo_argv1.tmp') or die "Can't open temp file: $!";
print while <TRY>;
open(TRY, '<tmpIo_argv2.tmp') or die "Can't open temp file: $!";
print while <TRY>;
close TRY or die "Could not close: $!";
undef $^I;

ok( eof TRY );

{
    no warnings 'once';
    ok( eof NEVEROPENED,    'eof() true on unopened filehandle' );
}

open STDIN, 'tmpIo_argv1.tmp' or die $!;
@ARGV = ();
ok( !eof(),     'STDIN has something' );

is( <>, "ok 11\n" );

SKIP: {
    skip_if_miniperl($no_devnull, 4);
    open STDIN, $devnull or die $!;
    @ARGV = ();
    ok( eof(),      'eof() true with empty @ARGV' );

    @ARGV = ('tmpIo_argv1.tmp');
    ok( !eof() );

    @ARGV = ($devnull, $devnull);
    ok( !eof() );

    close ARGV or die $!;
    ok( eof(),      'eof() true after closing ARGV' );
}

SKIP: {
    local $/;
    open my $fh, 'tmpIo_argv1.tmp' or die "Could not open tmpIo_argv1.tmp: $!";
    <$fh>;	# set $. = 1
    is( <$fh>, undef );

    skip_if_miniperl($no_devnull, 5);

    open $fh, $devnull or die;
    ok( defined(<$fh>) );

    is( <$fh>, undef );
    is( <$fh>, undef );

    open $fh, $devnull or die;	# restart cycle again
    ok( defined(<$fh>) );
    is( <$fh>, undef );
    close $fh or die "Could not close: $!";
}

open(TRY, '>tmpIo_argv1.tmp') || (die "Can't open temp file: $!");
print TRY "one\n\nthree\n";
close TRY or die "Could not close: $!";

$x = runperl(
    prog	=> 'print $..$ARGV.$_ while <<>>',
    args	=> [ 'tmpIo_argv1.tmp' ],
);
is($x, "1tmpIo_argv1.tmpone\n2tmpIo_argv1.tmp\n3tmpIo_argv1.tmpthree\n", '<<>>');

$x = runperl(
    prog	=> '$w=q/b/;$w.=<<>>;print $w',
    args	=> [ 'tmpIo_argv1.tmp' ],
);
is($x, "bone\n", '<<>> and rcatline');

$x = runperl(
    prog	=> 'while (<<>>) { print }',
    stdin	=> "foo\n",
);
is($x, "foo\n", '<<>> from just STDIN (no argument)');

TODO: {
    local $::TODO = "unrelated bug in redirection implementation" if $^O eq 'VMS';
    $x = runperl(
        prog	=> 'print $ARGV.q/,/ for <<>>',
        stdin	=> "foo\nbar\n",
    );
    is($x, "-,-,", '$ARGV is - for STDIN with <<>>');
}

$x = runperl(
    prog	=> 'while (<<>>) { print $_; }',
    stdin	=> "foo\n",
    stderr	=> 1,
    args	=> [ '-' ],
);
like($x, qr/^Can't open -: .* at -e line 1/, '<<>> does not treat - as STDIN');

{
    # tests for an empty string in @ARGV
    $x = runperl(
        prog	=> 'push @ARGV,q//;print while <>',
        stderr	=> 1,
    );
    like($x, qr/^Can't open : .* at -e line 1/, '<> does not open empty string in ARGV');

    $x = runperl(
        prog	=> 'push @ARGV,q//;print while <<>>',
        stderr	=> 1,
    );
    like($x, qr/^Can't open : .* at -e line 1/, '<<>> does not open empty string in ARGV');
}

SKIP: {
    skip('no echo', 2) unless -x '/bin/echo';

    $x = runperl(
        prog	=> 'while (<<>>) { print $_; }',
        stderr	=> 1,
        args	=> [ '"echo foo |"' ],
    );
    like($x, qr/^Can't open echo foo \|: .* at -e line 1/, '<<>> does not treat ...| as fork');

    $x = runperl(
        prog	=> 'while (<<>>) { }',
        stderr	=> 1,
        args	=> [ 'tmpIo_argv1.tmp', '"echo foo |"' ],
    );
    like($x, qr/^Can't open echo foo \|: .* at -e line 1, <> line 3/, '<<>> does not treat ...| as fork after eof');
}

# This used to dump core
fresh_perl_is( <<'**PROG**', "foobar", {}, "ARGV aliasing and eof()" ); 
open OUT, ">tmpIo_argv3.tmp" or die "Can't open temp file: $!";
print OUT "foo";
close OUT;
open IN, "tmpIo_argv3.tmp" or die "Can't open temp file: $!";
*ARGV = *IN;
while (<>) {
    print;
    print "bar" if eof();
}
close IN;
unlink "tmpIo_argv3.tmp";
**PROG**

# This used to fail an assertion.
# The tricks with *x and $x are to make PL_argvgv point to a freed SV when
# the readline op does SvREFCNT_inc on it.  undef *x clears the scalar slot
# ++$x vivifies it, reusing the just-deleted GV that PL_argvgv still points
# to.  The BEGIN block ensures it is freed late enough that nothing else
# has reused it yet.
is runperl(prog => 'undef *x; delete $::{ARGV}; $x++;'
                  .'eval q-BEGIN{undef *x} readline-; print qq-ok\n-'),
  "ok\n", 'deleting $::{ARGV}';

END {
    unlink_all 'tmpIo_argv1.tmp', 'tmpIo_argv1.tmp_bak',
	'tmpIo_argv2.tmp', 'tmpIo_argv2.tmp_bak', 'tmpIo_argv3.tmp';
}
