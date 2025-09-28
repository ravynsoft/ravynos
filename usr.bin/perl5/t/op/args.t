#!./perl

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}

plan( tests => 23 );

# test various operations on @_

sub new1 { bless \@_ }
{
    my $x = new1("x");
    my $y = new1("y");
    is("@$y","y", 'bless');
    is("@$x","x", 'bless');
}

sub new2 { splice @_, 0, 0, "a", "b", "c"; return \@_ }
{
    my $x = new2("x");
    my $y = new2("y");
    is("@$x","a b c x", 'splice');
    is("@$y","a b c y", 'splice');
}

sub new3 { goto &new1 }
{
    my $x = new3("x");
    my $y = new3("y");
    is("@$y","y", 'goto: single element');
    is("@$x","x", 'goto: single element');
}

sub new4 { goto &new2 }
{
    my $x = new4("x");
    my $y = new4("y");
    is("@$x","a b c x", 'goto: multiple elements');
    is("@$y","a b c y", 'goto: multiple elements');
}

# see if cx_popsub() gets to see the right pad across a dounwind() with
# a reified @_

sub methimpl {
    my $refarg = \@_;
    die( "got: @_\n" );
}

sub method {
    &methimpl;
}

my $failcount = 0;
sub try {
    eval { method('foo', 'bar'); };
    print "# $@" if $@;
    $failcount++;
}

for (1..5) { try() }
is($failcount, 5,
    'cx_popsub sees right pad across a dounwind() with reified @_');

# bug #21542 local $_[0] causes reify problems and coredumps

sub local1 { local $_[0] }
my $foo = 'foo'; local1($foo); local1($foo);
is($foo, 'foo',
    "got 'foo' as expected rather than '\$foo': RT \#21542");

sub local2 { local $_[0]; last L }
L: { local2 }
pass("last to label");

$|=1;

sub foo { local(@_) = ('p', 'q', 'r'); }
sub bar { unshift @_, 'D'; @_ }
sub baz { push @_, 'E'; return @_ }
for (1..3) { 
    is(join('',foo('a', 'b', 'c')),'pqr', 'local @_');
    is(join('',bar('d')),'Dd', 'unshift @_');
    is(join('',baz('e')),'eE', 'push @_');
} 

# [perl #28032] delete $_[0] was freeing things too early

{
    my $flag = 0;
    sub X::DESTROY { $flag = 1 }
    sub f {
	delete $_[0];
	ok(!$flag, 'delete $_[0] : in f');
    }
    {
	my $x = bless [], 'X';
	f($x);
	ok(!$flag, 'delete $_[0] : after f');
    }
    ok($flag, 'delete $_[0] : outside block');
}

	
