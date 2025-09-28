#!./perl -w

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
}

plan tests => 4;

use utf8;
use open qw( :utf8 :std );

sub goto_baresub {
    goto &問題の原因;
}

sub goto_softref {
    goto &{"問題の原因"};
}

sub goto_softref_octal {
    goto &{"\345\225\217\351\241\214\343\201\256\345\216\237\345\233\240"};
}

sub 問題の原因 {
    1;
}

ok goto_baresub(), "Magical goto works on an UTF-8 sub,";
ok goto_softref(), "..and an UTF-8 softref sub,";

{
    local $@;
    eval { goto_softref_octal() };
    like $@, qr/Goto undefined subroutine &main::\345\225\217\351\241\214\343\201\256\345\216\237\345\233\240/, "But does NOT find the softref sub when it's lacking the UTF-8 flag";
}

{
    local $@;
    eval { goto &因 };
    like $@, qr/Goto undefined subroutine &main::因/, "goto undefined sub gets the right error message";
}
