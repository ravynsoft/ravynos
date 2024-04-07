#!/usr/bin/perl -Tw

BEGIN {
    if( $ENV{PERL_CORE} ) {
        @INC = '../lib';
        chdir 't';
    }
}

use Test::More;

my $ro_err = qr/^Modification of a read-only value attempted/;

### Read-only scalar
my $foo;

ok( !Internals::SvREADONLY $foo );
$foo = 3;
is($foo, 3);

ok(  Internals::SvREADONLY $foo, 1 );
ok(  Internals::SvREADONLY $foo );
eval { $foo = 'foo'; };
like($@, $ro_err, q/Can't modify read-only scalar/);
eval { undef($foo); };
like($@, $ro_err, q/Can't undef read-only scalar/);
is($foo, 3);

ok( !Internals::SvREADONLY $foo, 0 );
ok( !Internals::SvREADONLY $foo );
$foo = 'foo';
is($foo, 'foo');

### Read-only array
my @foo;

ok( !Internals::SvREADONLY @foo );
@foo = (1..3);
is(scalar(@foo), 3);
is($foo[2], 3);

ok(  Internals::SvREADONLY @foo, 1 );
ok(  Internals::SvREADONLY @foo );
eval { undef(@foo); };
like($@, $ro_err, q/Can't undef read-only array/);
eval { delete($foo[2]); };
like($@, $ro_err, q/Can't delete from read-only array/);
eval { shift(@foo); };
like($@, $ro_err, q/Can't shift read-only array/);
eval { push(@foo, 'bork'); };
like($@, $ro_err, q/Can't push onto read-only array/);
eval { @foo = qw/foo bar/; };
like($@, $ro_err, q/Can't reassign read-only array/);

ok( !Internals::SvREADONLY @foo, 0 );
ok( !Internals::SvREADONLY @foo );
eval { @foo = qw/foo bar/; };
is(scalar(@foo), 2);
is($foo[1], 'bar');

### Read-only array element

ok( !Internals::SvREADONLY $foo[2] );
$foo[2] = 'baz';
is($foo[2], 'baz');

ok(  Internals::SvREADONLY $foo[2], 1 );
ok(  Internals::SvREADONLY $foo[2] );

$foo[0] = 99;
is($foo[0], 99, 'Rest of array still modifiable');

shift(@foo);
ok(  Internals::SvREADONLY $foo[1] );
eval { $foo[1] = 'bork'; };
like($@, $ro_err, 'Read-only array element moved');
is($foo[1], 'baz');

ok( !Internals::SvREADONLY $foo[2] );
$foo[2] = 'qux';
is($foo[2], 'qux');

unshift(@foo, 'foo');
ok( !Internals::SvREADONLY $foo[1] );
ok(  Internals::SvREADONLY $foo[2] );

eval { $foo[2] = 86; };
like($@, $ro_err, q/Can't modify read-only array element/);
eval { undef($foo[2]); };
like($@, $ro_err, q/Can't undef read-only array element/);
TODO: {
    local $TODO = 'Due to restricted hashes implementation';
    eval { delete($foo[2]); };
    like($@, $ro_err, q/Can't delete read-only array element/);
}

ok( !Internals::SvREADONLY $foo[2], 0 );
ok( !Internals::SvREADONLY $foo[2] );
$foo[2] = 'xyzzy';
is($foo[2], 'xyzzy');

### Read-only hash
my %foo;

ok( !Internals::SvREADONLY %foo );
%foo = ('foo' => 1, 2 => 'bar');
is(scalar(keys(%foo)), 2);
is($foo{'foo'}, 1);

ok(  Internals::SvREADONLY %foo, 1 );
ok(  Internals::SvREADONLY %foo );
eval { undef(%foo); };
like($@, $ro_err, q/Can't undef read-only hash/);
TODO: {
    local $TODO = 'Due to restricted hashes implementation';
    eval { %foo = ('ping' => 'pong'); };
    like($@, $ro_err, q/Can't modify read-only hash/);
}
eval { $foo{'baz'} = 123; };
like($@, qr/Attempt to access disallowed key/, q/Can't add to a read-only hash/);

# These ops are allow for Hash::Util functionality
$foo{2} = 'qux';
is($foo{2}, 'qux', 'Can modify elements in a read-only hash');
my $qux = delete($foo{2});
ok(! exists($foo{2}), 'Can delete keys from a read-only hash');
is($qux, 'qux');
$foo{2} = 2;
is($foo{2}, 2, 'Can add back deleted keys in a read-only hash');

ok( !Internals::SvREADONLY %foo, 0 );
ok( !Internals::SvREADONLY %foo );

### Read-only hash values

ok( !Internals::SvREADONLY $foo{foo} );
$foo{'foo'} = 'bar';
is($foo{'foo'}, 'bar');

ok(  Internals::SvREADONLY $foo{foo}, 1 );
ok(  Internals::SvREADONLY $foo{foo} );
eval { $foo{'foo'} = 88; };
like($@, $ro_err, q/Can't modify a read-only hash value/);
eval { undef($foo{'foo'}); };
like($@, $ro_err, q/Can't undef a read-only hash value/);
my $bar = delete($foo{'foo'});
ok(! exists($foo{'foo'}), 'Can delete a read-only hash value');
is($bar, 'bar');

ok( !Internals::SvREADONLY $foo{foo}, 0 );
ok( !Internals::SvREADONLY $foo{foo} );

is(  Internals::SvREFCNT($foo), 1 );
{
    my $bar = \$foo;
    is(  Internals::SvREFCNT($foo), 2 );
    is(  Internals::SvREFCNT($bar), 1 );
}
is(  Internals::SvREFCNT($foo), 1 );

is(  Internals::SvREFCNT(@foo), 1 );
is(  Internals::SvREFCNT($foo[2]), 1 );
is(  Internals::SvREFCNT(%foo), 1 );
is(  Internals::SvREFCNT($foo{foo}), 1 );

is(  Internals::SvREFCNT($foo, 2), 2, "update ref count");
is(  Internals::SvREFCNT($foo), 2, "check we got the stored value");

# the reference count is a U16, but was returned as an IV resulting in
# different values between 32 and 64-bit builds
my $big_count = 0xFFFFFFF0; # -16 32-bit signed
is( Internals::SvREFCNT($foo, $big_count), $big_count,
    "set reference count unsigned");
is( Internals::SvREFCNT($foo), $big_count, "reference count unsigned");

{
    my @arr = Internals::SvREFCNT($foo, 1 );
    is(scalar(@arr), 1, "SvREFCNT always returns only 1 item");
}

{
    my $usage =  'Usage: Internals::SvREFCNT(SCALAR[, REFCOUNT])';
    eval { &Internals::SvREFCNT();};
    like($@, qr/\Q$usage\E/);
    $foo = \"perl";
    eval { &Internals::SvREFCNT($foo, 0..1);};
    like($@, qr/\Q$usage\E/);
    eval { &Internals::SvREFCNT($foo, 0..3);};
    like($@, qr/\Q$usage\E/);
}

done_testing();
