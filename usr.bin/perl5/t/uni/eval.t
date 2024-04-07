#!./perl

# Check if eval correctly ignores the UTF-8 hint.

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
}

plan (tests => 5);

use open qw( :utf8 :std );
use feature 'unicode_eval';

{
    my $w;
    $SIG{__WARN__} = sub { $w = shift };
    use utf8;
    my $prog = "qq!\x{f9}!";

    eval $prog;
    ok !$w;

    $w = "";
    utf8::upgrade($prog);
    eval $prog;
    is $w, '';
}

{
    use utf8;
    isnt eval "q!\360\237\220\252!", eval "q!\x{1f42a}!";
}

{
    no utf8; #Let's make real sure.
    my $not_utf8 = "q!\343\203\213!";
    isnt eval $not_utf8, eval "q!\x{30cb}!";
    {
        use utf8;
        isnt eval $not_utf8, eval "q!\x{30cb}!";
    }
}
