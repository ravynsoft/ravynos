#!./perl
use warnings;
use strict;

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    require './charset_tools.pl';
    set_up_inc('../lib');
}

plan (tests => 46 + 3 * $::IS_ASCII);

is(length(""), 0);
is(length("abc"), 3);
$_ = "foobar";
is(length(), 6);

# Okay, so that wasn't very challenging.  Let's go Unicode.

{
    my $a = "\x{41}";
    is(length($a), 1);

    use bytes;
    ok($a eq "\x41");
    is(length($a), 1);
}

if ($::IS_ASCII) {  # Generally UTF-8 invariant on EBCDIC, so skip there
    my $a = pack("U", 0xFF);

    is(length($a), 1);

    use bytes;
    ok($a eq byte_utf8a_to_utf8n("\xc3\xbf"));
    is(length($a), 2);
}

{
    my $a = pack("U", 0xB6);    # Works on both ASCII and EBCDIC

    is(length($a), 1);

    use bytes;
    ok($a eq byte_utf8a_to_utf8n("\xc2\xb6"));
    is(length($a), 2);
}

{
    my $a = "\x{100}";

    is(length($a), 1);

    use bytes;
    ok($a eq byte_utf8a_to_utf8n("\xc4\x80"));
    is(length($a), 2);
}

{
    my $a = "\x{100}\x{B6}";

    is(length($a), 2);

    use bytes;
    ok($a eq byte_utf8a_to_utf8n("\xc4\x80\xc2\xb6"));
    is(length($a), 4);
}

{
    my $a = "\x{b6}\x{100}";

    is(length($a), 2);

    use bytes;
    ok($a eq byte_utf8a_to_utf8n("\xc2\xb6\xc4\x80"));
    is(length($a), 4);
}

# Now for Unicode with magical vtbls

{
    require Tie::Scalar;
    my $a;
    tie $a, 'Tie::StdScalar';  # makes $a magical
    $a = "\x{263A}";

    is(length($a), 1);

    use bytes;
    is(length($a), 3);
}

{
    # Play around with Unicode strings,
    # give a little workout to the UTF-8 length cache.
    my $a = chr(256) x 100;
    is(length $a, 100);
    chop $a;
    is(length $a, 99);
    $a .= $a;
    is(length $a, 198);
    $a = chr(256) x 999;
    is(length $a, 999);
    substr($a, 0, 1) = '';
    is(length $a, 998);
}

require Tie::Scalar;

my $u = "ASCII";

tie $u, 'Tie::StdScalar', chr 256;

is(length $u, 1, "Length of a UTF-8 scalar returned from tie");
is(length $u, 1, "Again! Again!");

$^W = 1;

my $warnings = 0;

$SIG{__WARN__} = sub {
    $warnings++;
    warn @_;
};

is(length(undef), undef, "Length of literal undef");

undef $u;

is(length($u), undef, "Length of regular scalar");

$u = "Gotcha!";

tie $u, 'Tie::StdScalar';

is(length($u), undef, "Length of tied scalar (MAGIC)");

is($u, undef);

{
    package U;
    use overload '""' => sub {return undef;};
}

my $uo = bless [], 'U';

{
    my $w;
    local $SIG{__WARN__} = sub { $w = shift };
    is(length($uo), 0, "Length of overloaded reference");
    like $w, qr/uninitialized/, 'uninit warning for stringifying as undef';
}

my $ul = 3;
is(($ul = length(undef)), undef,
                    "Returned length of undef with result in TARG");
is($ul, undef, "Assigned length of undef with result in TARG");

$ul = 3;
is(($ul = length($u)), undef,
                "Returned length of tied undef with result in TARG");
is($ul, undef, "Assigned length of tied undef with result in TARG");

$ul = 3;
{
    my $w;
    local $SIG{__WARN__} = sub { $w = shift };
    is(($ul = length($uo)), 0,
                "Returned length of overloaded undef with result in TARG");
    like $w, qr/uninitialized/, 'uninit warning for stringifying as undef';
}
is($ul, 0, "Assigned length of overloaded undef with result in TARG");

{
    my $y = "\x{100}BC";
    is(index($y, "B"), 1, 'adds an intermediate position to the offset cache');
    is(length $y, 3,
       'Check that sv_len_utf8() can take advantage of the offset cache');
}

{
    local $SIG{__WARN__} = sub {
        pass("'print length undef' warned");
    };
    print length undef;
}

{
    local $SIG{__WARN__} = sub {
	pass '[perl #106726] no crash with length @lexical warning'
    };
    eval ' sub { length my @forecasts } ';
}

# length could be fooled by UTF8ness of non-magical variables changing with
# stringification.
my $ref = [];
bless $ref, "\x{100}";
is length $ref, length "$ref", 'length on reference blessed to utf8 class';

is($warnings, 0, "There were no other warnings");
