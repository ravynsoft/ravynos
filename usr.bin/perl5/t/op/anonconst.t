#!./perl

BEGIN {
    chdir 't';
    require './test.pl';
    set_up_inc("../lib");
}

plan 8;

{
    my $w;
    local $SIG{__WARN__} = sub { $w .= shift };
    eval '+sub : const {}';
    like $w, qr/^:const is experimental at /, 'experimental warning';
}

no warnings 'experimental::const_attr';

push @subs, sub :const{$_} for 1..10;
is join(" ", map &$_, @subs), "1 2 3 4 5 6 7 8 9 10",
  ':const capturing global $_';

my $x = 3;
my $sub = sub : const { $x };
$x++;
is &$sub, 3, ':const capturing lexical';

$x = 3;
$sub = sub : const { $x+5 };
$x++;
is &$sub, 8, ':const capturing expression';

is &{sub () : const { 42 }}, 42, ':const with truly constant sub';

*foo = $sub;
{
    use warnings 'redefine';
    my $w;
    local $SIG{__WARN__} = sub { $w .= shift };
    *foo = sub (){};
    like $w, qr/^Constant subroutine main::foo redefined at /,
        ':const subs are constant';
}

eval 'sub bar : const';
like $@, qr/^:const is not permitted on named subroutines at /,
    ':const on named stub';
eval 'sub baz : const { }';
like $@, qr/^:const is not permitted on named subroutines at /,
    ':const on named sub';
