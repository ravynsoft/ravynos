#!./perl
# Tests for caller()

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
    plan( tests => 111 ); # some tests are run in a BEGIN block
}

my @c;

BEGIN { print "# Tests with caller(0)\n"; }

@c = caller(0);
ok( (!@c), "caller(0) in main program" );

eval { @c = caller(0) };
is( $c[3], "(eval)", "subroutine name in an eval {}" );
ok( !$c[4], "hasargs false in an eval {}" );

eval q{ @c = caller(0) };
is( $c[3], "(eval)", "subroutine name in an eval ''" );
ok( !$c[4], "hasargs false in an eval ''" );

sub { @c = caller(0) } -> ();
is( $c[3], "main::__ANON__", "anonymous subroutine name" );
ok( $c[4], "hasargs true with anon sub" );

# Bug 20020517.003 (#9367), used to dump core
sub foo { @c = caller(0) }
my $fooref = delete $::{foo};
$fooref -> ();
is( $c[3], "main::foo", "deleted subroutine name" );
ok( $c[4], "hasargs true with deleted sub" );

BEGIN {
 require strict;
 is +(caller 0)[1], __FILE__,
  "[perl #68712] filenames after require in a BEGIN block"
}

print "# Tests with caller(1)\n";

sub f { @c = caller(1) }

sub callf { f(); }
callf();
is( $c[3], "main::callf", "subroutine name" );
ok( $c[4], "hasargs true with callf()" );
&callf;
ok( !$c[4], "hasargs false with &callf" );

eval { f() };
is( $c[3], "(eval)", "subroutine name in an eval {}" );
ok( !$c[4], "hasargs false in an eval {}" );

eval q{ f() };
is( $c[3], "(eval)", "subroutine name in an eval ''" );
ok( !$c[4], "hasargs false in an eval ''" );

sub { f() } -> ();
is( $c[3], "main::__ANON__", "anonymous subroutine name" );
ok( $c[4], "hasargs true with anon sub" );

sub foo2 { f() }
my $fooref2 = delete $::{foo2};
$fooref2 -> ();
is( $c[3], "main::foo2", "deleted subroutine name" );
ok( $c[4], "hasargs true with deleted sub" );

# See if caller() returns the correct warning mask

sub show_bits
{
    my $in = shift;
    my $out = '';
    foreach (unpack('W*', $in)) {
        $out .= sprintf('\x%02x', $_);
    }
    return $out;
}

sub check_bits
{
    local $Level = $Level + 2;
    my ($got, $exp, $desc) = @_;
    if (! ok($got eq $exp, $desc)) {
        diag('     got: ' . show_bits($got));
        diag('expected: ' . show_bits($exp));
    }
}

sub testwarn {
    my $w = shift;
    my $id = shift;
    check_bits( (caller(0))[9], $w, "warnings match caller ($id)");
}

{
    no warnings;
    BEGIN { check_bits( ${^WARNING_BITS}, "\0" x $warnings::BYTES, 'all bits off via "no warnings"' ) }
    testwarn("\0" x $warnings::BYTES, 'no bits');

    use warnings;
    BEGIN { check_bits( ${^WARNING_BITS}, "\x55" x $warnings::BYTES,
			'default bits on via "use warnings"' ); }
    testwarn("\x55" x $warnings::BYTES, 'all');
}


# The next two cases test for a bug where caller ignored evals if
# the DB::sub glob existed but &DB::sub did not (for example, if 
# $^P had been set but no debugger has been loaded).  The tests
# thus assume that there is no &DB::sub: if there is one, they 
# should both pass  no matter whether or not this bug has been
# fixed.

my $debugger_test =  q<
    my @stackinfo = caller(0);
    return scalar @stackinfo;
>;

sub pb { return (caller(0))[3] }

my $i = eval $debugger_test;
is( $i, 11, "do not skip over eval (and caller returns 10 elements)" );

is( eval 'pb()', 'main::pb', "actually return the right function name" );

my $saved_perldb = $^P;
$^P = 16;
$^P = $saved_perldb;

$i = eval $debugger_test;
is( $i, 11, 'do not skip over eval even if $^P had been on at some point' );
is( eval 'pb()', 'main::pb', 'actually return the right function name even if $^P had been on at some point' );

print "# caller can now return the compile time state of %^H\n";

sub hint_exists {
    my $key = shift;
    my $level = shift;
    my @results = caller($level||0);
    exists $results[10]->{$key};
}

sub hint_fetch {
    my $key = shift;
    my $level = shift;
    my @results = caller($level||0);
    $results[10]->{$key};
}

{
    my $tmpfile = tempfile();

    open my $fh, '>', $tmpfile or die "open $tmpfile: $!";
    print $fh <<'EOP';
#!perl -wl
use strict;

{
    package KAZASH ;

    sub DESTROY {
	print "DESTROY";
    }
}

@DB::args = bless [], 'KAZASH';

print $^P;
print scalar @DB::args;

{
    local $^P = shift;
}

@DB::args = (); # At this point, the object should be freed.

print $^P;
print scalar @DB::args;

# It shouldn't leak.
EOP
    close $fh;

    foreach (0, 1) {
        my $got = runperl(progfile => $tmpfile, args => [$_]);
        $got =~ s/\s+/ /gs;
        like($got, qr/\s*0 1 DESTROY 0 0\s*/,
             "\@DB::args doesn't leak with \$^P = $_");
    }
}

# This also used to leak [perl #97010]:
{
    my $gone;
    sub fwib::DESTROY { ++$gone }
    package DB;
    sub { () = caller(0) }->(); # initialise PL_dbargs
    @args = bless[],'fwib';
    sub { () = caller(0) }->(); # clobber @args without initialisation
    ::is $gone, 1, 'caller does not leak @DB::args elems when AvREAL';
}

# And this crashed [perl #93320]:
sub {
  package DB;
  ()=caller(0);
  undef *DB::args;
  ()=caller(0);
}->();
pass 'No crash when @DB::args is freed between caller calls';

# This also crashed:
package glelp;
sub TIEARRAY { bless [] }
sub EXTEND   {         }
sub CLEAR    {        }
sub FETCH    { $_[0][$_[1]] }
sub STORE    { $_[0][$_[1]] = $_[2] }
package DB;
tie @args, 'glelp';
eval { sub { () = caller 0; } ->(1..3) };
::like $@, qr "^Cannot set tied \@DB::args at ",
              'caller dies with tie @DB::args';
::ok tied @args, '@DB::args is still tied';
untie @args;
package main;

# [perl #113486]
fresh_perl_is <<'END', "ok\n", {},
  { package foo; sub bar { main::bar() } }
  sub bar {
    delete $::{"foo::"};
    my $x = \($1+2);
    my $y = \($1+2); # this is the one that reuses the mem addr, but
    my $z = \($1+2);  # try the others just in case
    s/2// for $$x, $$y, $$z; # now SvOOK
    $x = caller;
    print "ok\n";
};
foo::bar
END
    "No crash when freed stash is reused for PV with offset hack";

is eval "(caller 0)[6]", "(caller 0)[6]",
  'eval text returned by caller does not include \n;';

if (1) {
    is (sub { (caller)[2] }->(), __LINE__,
      '[perl #115768] caller gets line numbers from nulled cops');
}
# Test it at the end of the program, too.
fresh_perl_is(<<'115768', 2, {},
  if (1) {
    foo();
  }
  sub foo { print +(caller)[2] }
115768
    '[perl #115768] caller gets line numbers from nulled cops (2)');

# PL_linestr should not be modifiable
eval '"${;BEGIN{  ${\(caller 2)[6]} = *foo  }}"';
pass "no assertion failure after modifying eval text via caller";

is eval "<<END;\nfoo\nEND\n(caller 0)[6]",
        "<<END;\nfoo\nEND\n(caller 0)[6]",
        'here-docs do not gut eval text';
is eval "s//<<END/e;\nfoo\nEND\n(caller 0)[6]",
        "s//<<END/e;\nfoo\nEND\n(caller 0)[6]",
        'here-docs in quote-like ops do not gut eval text';

# The bitmask should be assignable to ${^WARNING_BITS} without resulting in
# different warnings settings.
{
 my $ bits = sub { (caller 0)[9] }->();
 my $w;
 local $SIG{__WARN__} = sub { $w++ };
 eval '
   use warnings;
   BEGIN { ${^WARNING_BITS} = $bits }
   local $^W = 1;
   () = 1 + undef;
   $^W = 0;
   () = 1 + undef;
 ';
 is $w, 1, 'value from (caller 0)[9] (bitmask) works in ${^WARNING_BITS}';
}

# [perl #126991]
sub getlineno { (caller)[2] }
my $line = eval "\n#line 3000000000\ngetlineno();";
is $line, "3000000000", "check large line numbers are preserved";

# This was fixed with commit d4d03940c58a0177, which fixed bug #78742
fresh_perl_is <<'END', "__ANON__::doof\n", {},
package foo;
BEGIN {undef %foo::}
sub doof { caller(0) }
print +(doof())[3];
END
    "caller should not SEGV when the current package is undefined";

# caller should not SEGV when the eval entry has been cleared #120998
fresh_perl_is <<'END', 'main', {},
$SIG{__DIE__} = \&dbdie;
eval '/x';
sub dbdie {
    @x = caller(1);
    print $x[0];
}
END
    "caller should not SEGV for eval '' stack frames";

TODO: {
    local $::TODO = 'RT #7165: line number should be consistent for multiline subroutine calls';
    fresh_perl_is(<<'EOP', "6\n9\n", {}, 'RT #7165: line number should be consistent for multiline subroutine calls');
      sub tagCall {
        my ($package, $file, $line) = caller;
        print "$line\n";
      }
      
      tagCall
      "abc";
      
      tagCall
      sub {};
EOP
}

$::testing_caller = 1;

do './op/caller.pl' or die $@;

# GH #15109
# See that callers within a nested series of 'use's gets the right
# filenames.
{
    local @INC = 'lib/caller/';
    # Apack use's Bpack which use's Cpack which populates @Cpack::caller
    # with the file:N of all the callers
    eval 'use Apack; 1';
    is($@, "", "GH #15109 - eval");
    is (scalar(@Cpack::callers), 10, "GH #15109 - callers count");
    like($Cpack::callers[$_], qr{caller/Bpack.pm:3}, "GH #15109 level $_") for 0..2;
    like($Cpack::callers[$_], qr{caller/Apack.pm:3}, "GH #15109 level $_") for 3..5;
    like($Cpack::callers[$_], qr{\(eval \d+\):1}, "GH #15109 level $_") for 6..8;
    like($Cpack::callers[$_], qr{caller\.t}, "GH #15109 level $_") for 9;

    # GH #15109 followup - the original fix wasn't saving cop_warnings
    # correctly and this code used to crash or fail valgrind

    my $w = 0;
    local $SIG{__WARN__} = sub { $w++ };
    eval q{
        use warnings;
        no warnings 'numeric'; # ensure custom cop_warnings
        use Foo;      # this used to mess up warnings flags
        BEGIN { my $x = "foo" + 1; } # potential "numeric" warning
    };
    is ($@, "", "GH #15109 - eval okay");
    is ($w, 0, "GH #15109 - warnings restored");
}

{
    package RT129239;
    BEGIN {
        my ($pkg, $file, $line) = caller;
        ::is $file, 'virtually/op/caller.t', "BEGIN block sees correct caller filename";
        ::is $line, 12345,                   "BEGIN block sees correct caller line";
        ::is $pkg, 'RT129239',               "BEGIN block sees correct caller package";
#line 12345 "virtually/op/caller.t"
    }

}
