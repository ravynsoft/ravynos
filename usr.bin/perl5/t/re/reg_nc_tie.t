#!./perl

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
    skip_all_if_miniperl("no dynamic loading on miniperl, no Tie::Hash::NamedCapture");
}

# Do a basic test on all the tied methods of Tie::Hash::NamedCapture

plan(tests => 37);

# PL_curpm->paren_names can be a null pointer. See that this succeeds anyway.
'x' =~ /(.)/;
() = %+;
pass( 'still alive' );

"hlagh" =~ /
    (?<a>.)
    (?<b>.)
    (?<a>.)
    .*
    (?<e>$)
/x;

# FETCH
is($+{a}, "h", "FETCH");
is($+{b}, "l", "FETCH");
is($-{a}[0], "h", "FETCH");
is($-{a}[1], "a", "FETCH");

# STORE
eval { $+{a} = "yon" };
like($@, qr/read-only/, "STORE");

# DELETE
eval { delete $+{a} };
like($@, qr/read-only/, "DELETE");

# CLEAR
eval { %+ = () };
like($@, qr/read-only/, "CLEAR");

# EXISTS
ok(exists $+{e}, "EXISTS");
ok(!exists $+{d}, "EXISTS");

# FIRSTKEY/NEXTKEY
is(join('|', sort keys %+), "a|b|e", "FIRSTKEY/NEXTKEY");

# SCALAR
is(scalar(%+), 3, "SCALAR");
is(scalar(%-), 3, "SCALAR");

# Abuse all methods with undef as the first argument (RT #71828 and then some):

is(Tie::Hash::NamedCapture::FETCH(undef, undef), undef, 'FETCH with undef');
eval {Tie::Hash::NamedCapture::STORE(undef, undef, undef)};
like($@, qr/Modification of a read-only value attempted/, 'STORE with undef');
eval {Tie::Hash::NamedCapture::DELETE(undef, undef)};
like($@, , qr/Modification of a read-only value attempted/,
     'DELETE with undef');
eval {Tie::Hash::NamedCapture::CLEAR(undef)};
like($@, qr/Modification of a read-only value attempted/, 'CLEAR with undef');
is(Tie::Hash::NamedCapture::EXISTS(undef, undef), undef, 'EXISTS with undef');
is(Tie::Hash::NamedCapture::FIRSTKEY(undef), undef, 'FIRSTKEY with undef');
is(Tie::Hash::NamedCapture::NEXTKEY(undef, undef), undef, 'NEXTKEY with undef');
is(Tie::Hash::NamedCapture::SCALAR(undef), undef, 'SCALAR with undef');

my $obj = tied %+;
foreach ([FETCH => '$key'],
	 [STORE => '$key, $value'],
	 [DELETE => '$key'],
	 [CLEAR => ''],
	 [EXISTS => '$key'],
	 [FIRSTKEY => ''],
	 [NEXTKEY => '$lastkey'],
	 [SCALAR => ''],
	) {
    my ($method, $error) = @$_;

    is(eval {$obj->$method(0..3); 1}, undef, "$method with undef");
    like($@, qr/Usage: Tie::Hash::NamedCapture::$method\(\Q$error\E\)/,
	 "usage method for $method");
}
