#!perl -w
use strict;

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
}

no warnings 'experimental::builtin';
use builtin 'weaken';

plan(tests => 14);

# [perl 72922]: A 'copy' of a Regex object which has magic should not crash
# When a Regex object was copied and the copy weaken then the original regex object
# could no longer be 'copied' with qr//

sub s1 {
    my $re = qr/abcdef/;
    my $re_copy1 = $re;
    my $re_weak_copy = $re;;
    weaken($re_weak_copy);
    my $re_copy2 = qr/$re/;

    my $str_re = "$re";
    is("$$re_weak_copy", $str_re, "weak copy equals original");
    is("$re_copy1", $str_re, "copy1 equals original");
    is("$re_copy2", $str_re, "copy2 equals original");

    my $refcnt_start = Internals::SvREFCNT($$re_weak_copy);

    undef $re;
    refcount_is $re_weak_copy, $refcnt_start - 1, "refcnt decreased";
    is("$re_weak_copy", $str_re, "weak copy still equals original");

    undef $re_copy2;
    refcount_is $re_weak_copy, $refcnt_start - 1, "refcnt not decreased";
    is("$re_weak_copy", $str_re, "weak copy still equals original");
}
s1();
s1();
