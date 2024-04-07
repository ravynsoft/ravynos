#!./perl

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
    require './charset_tools.pl';
}

plan(tests => 9);

{
    local $SIG{__WARN__} = sub {};
    eval "evalbytes 'foo'";
    like $@, qr/syntax error/, 'evalbytes outside feature scope';
}

# We enable unicode_eval just to test that it does not interfere.
use feature 'evalbytes', 'unicode_eval';

is evalbytes("1+7"), 8, 'evalbytes basic sanity check';

my $code = qq('\xff\xfe');
is evalbytes($code), "\xff\xfe", 'evalbytes on extra-ASCII bytes';
chop((my $upcode = $code) .= chr 256);
is evalbytes($upcode), "\xff\xfe", 'evalbytes on upgraded extra-ASCII';
{
    use utf8;
    is evalbytes($code), "\xff\xfe", 'evalbytes ignores outer utf8 pragma';
}
my $U_100 = byte_utf8a_to_utf8n("\xc4\x80");
is evalbytes "use utf8; $U_100", chr 256, 'use utf8 within evalbytes';
chop($upcode = "use utf8; $U_100" . chr 256);
is evalbytes $upcode, chr 256, 'use utf8 within evalbytes on utf8 string';
eval { evalbytes chr 256 };
like $@, qr/Wide character/, 'evalbytes croaks on non-bytes';

eval 'evalbytes S';
ok 1, '[RT #129196] evalbytes S should not segfault';

