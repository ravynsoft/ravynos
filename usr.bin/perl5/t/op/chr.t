#!./perl

BEGIN {
    chdir 't' if -d 't';
    require "./test.pl";
    set_up_inc(qw(. ../lib)); # ../lib needed for test.deparse
}

plan tests => 45;

# Note that t/op/ord.t already tests for chr() <-> ord() rountripping.

# Don't assume ASCII.

is(chr(ord("A")), "A");

is(chr(  0), "\x00");
is(chr(127), "\x7F");
is(chr(128), "\x80");
is(chr(255), "\xFF");

is(chr(-0.1), "\x{FFFD}"); # The U+FFFD Unicode replacement character.
is(chr(-1  ), "\x{FFFD}");
is(chr(-2  ), "\x{FFFD}");
is(chr(-3.0), "\x{FFFD}");
{
    use bytes; # Backward compatibility.
    is(chr(-0.1), "\x00");
    is(chr(-1  ), "\xFF");
    is(chr(-2  ), "\xFE");
    is(chr(-3.0), "\xFD");
}

# Make sure -1 is treated the same way when coming from a tied variable
sub TIESCALAR {bless[]}
sub STORE { $_[0][0] = $_[1] }
sub FETCH { $_[0][0] }
tie $t, "";
$t = -1; is chr $t, chr -1, 'chr $tied when $tied is -1';
$t = -2; is chr $t, chr -2, 'chr $tied when $tied is -2';
$t = -1.1; is chr $t, chr -1.1, 'chr $tied when $tied is -1.1';
$t = -2.2; is chr $t, chr -2.2, 'chr $tied when $tied is -2.2';

# And that stringy scalars are treated likewise
is chr "-1", chr -1, 'chr "-1" eq chr -1';
is chr "-2", chr -2, 'chr "-2" eq chr -2';
is chr "-1.1", chr -1.1, 'chr "-1.1" eq chr -1.1';
is chr "-2.2", chr -2.2, 'chr "-2.2" eq chr -2.2';

# Check UTF-8 (not UTF-EBCDIC).
SKIP: {
    skip "UTF-8 ASCII centric tests", 21 if $::IS_EBCDIC;
    # Too hard to convert these tests generically to EBCDIC code pages without
    # using chr(), which is what we're testing.

sub hexes {
    no warnings 'utf8'; # avoid surrogate and beyond Unicode warnings
    join(" ",unpack "U0 (H2)*", chr $_[0]);
}

# The following code points are some interesting steps in UTF-8.
    is(hexes(   0x100), "c4 80");
    is(hexes(   0x7FF), "df bf");
    is(hexes(   0x800), "e0 a0 80");
    is(hexes(   0xFFF), "e0 bf bf");
    is(hexes(  0x1000), "e1 80 80");
    is(hexes(  0xCFFF), "ec bf bf");
    is(hexes(  0xD000), "ed 80 80");
    is(hexes(  0xD7FF), "ed 9f bf");
    is(hexes(  0xD800), "ed a0 80"); # not strict utf-8 (surrogate area begin)
    is(hexes(  0xDFFF), "ed bf bf"); # not strict utf-8 (surrogate area end)
    is(hexes(  0xE000), "ee 80 80");
    is(hexes(  0xFFFF), "ef bf bf");
    is(hexes( 0x10000), "f0 90 80 80");
    is(hexes( 0x3FFFF), "f0 bf bf bf");
    is(hexes( 0x40000), "f1 80 80 80");
    is(hexes( 0xFFFFF), "f3 bf bf bf");
    is(hexes(0x100000), "f4 80 80 80");
    is(hexes(0x10FFFF), "f4 8f bf bf"); # Unicode (4.1) last code point
    is(hexes(0x110000), "f4 90 80 80");
    is(hexes(0x1FFFFF), "f7 bf bf bf"); # last four byte encoding
    is(hexes(0x200000), "f8 88 80 80 80");
}

package o {
    use overload
        '""' => sub { ++$o::str; "42" },
        '0+' => sub { ++$o::num; 42 };
}
is chr(bless [], "o"), chr(42), 'overloading called';
is $o::str, undef, 'chr does not call string overloading';
is $o::num, 1,     'chr does call num overloading';
