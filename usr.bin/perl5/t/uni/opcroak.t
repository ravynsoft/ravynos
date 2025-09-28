#!./perl

#
# tests for op.c generated croaks
#

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc(qw '../lib ../dist/base/lib');
}

use utf8;
use open qw( :utf8 :std );
use warnings;

plan( tests => 5 );

eval qq!sub \x{30cb} (\$) {} \x{30cb}()!;
like $@, qr/Not enough arguments for main::\x{30cb}/u, "Not enough arguments croak is UTF-8 clean";

eval qq!sub \x{30cc} (\$) {} \x{30cc}(1, 2)!;
like $@, qr/Too many arguments for main::\x{30cc}/u, "Too many arguments croak is UTF-8 clean";

eval qq!sub \x{30cd} (\Q\%\E) { 1 } \x{30cd}(1);!;
like $@, qr/Type of arg 1 to main::\x{30cd} must be/u, "bad type croak is UTF-8 clean";

    eval <<'END_FIELDS';
    {
        package ＦŌŌ {
            use fields qw( a b );
            sub new { bless {}, shift }
        }
    }
END_FIELDS

die $@ if $@;

for (
        [ element => 'my ＦŌŌ $bàr = ＦŌŌ->new; $bàr->{クラス};' ],
        [ slice => 'my ＦŌŌ $bàr = ＦŌŌ->new; @{$bàr}{ qw( a クラス ) };' ]
    ) {
    eval $_->[1];
    
    like $@, qr/No such class field "クラス" in variable \$bàr of type ＦŌŌ/, "$_->[0]: no such field error is UTF-8 clean";
}
