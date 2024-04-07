#!./perl

chdir 't' if -d 't';
@INC = qw(. ../lib);
require "./test.pl";
plan( tests => 64 );

$aa = 1;
{ local $aa;     $aa = 2; is($aa,2); }
is($aa,1);
{ local ${aa};   $aa = 3; is($aa,3); }
is($aa,1);
{ local ${"aa"}; $aa = 4; is($aa,4); }
is($aa,1);
$x = "aa";
{ local ${$x};   $aa = 5; is($aa,5); undef $x; is($aa,5); }
is($aa,1);
$x = "a";
{ local ${$x x2};$aa = 6; is($aa,6); undef $x; is($aa,6); }
is($aa,1);
$x = "aa";
{ local $$x;     $aa = 7; is($aa,7); undef $x; is($aa,7); }
is($aa,1);

@aa = qw/a b/;
{ local @aa;     @aa = qw/c d/; is("@aa","c d"); }
is("@aa","a b");
{ local @{aa};   @aa = qw/e f/; is("@aa","e f"); }
is("@aa","a b");
{ local @{"aa"}; @aa = qw/g h/; is("@aa","g h"); }
is("@aa","a b");
$x = "aa";
{ local @{$x};   @aa = qw/i j/; is("@aa","i j"); undef $x; is("@aa","i j"); }
is("@aa","a b");
$x = "a";
{ local @{$x x2};@aa = qw/k l/; is("@aa","k l"); undef $x; is("@aa","k l"); }
is("@aa","a b");
$x = "aa";
{ local @$x;     @aa = qw/m n/; is("@aa","m n"); undef $x; is("@aa","m n"); }
is("@aa","a b");

%aa = qw/a b/;
{ local %aa;     %aa = qw/c d/; is($aa{c},"d"); }
is($aa{a},"b");
{ local %{aa};   %aa = qw/e f/; is($aa{e},"f"); }
is($aa{a},"b");
{ local %{"aa"}; %aa = qw/g h/; is($aa{g},"h"); }
is($aa{a},"b");
$x = "aa";
{ local %{$x};   %aa = qw/i j/; is($aa{i},"j"); undef $x; is($aa{i},"j"); }
is($aa{a},"b");
$x = "a";
{ local %{$x x2};%aa = qw/k l/; is($aa{k},"l"); undef $x; is($aa{k},"l"); }
is($aa{a},"b");
$x = "aa";
{ local %$x;     %aa = qw/m n/; is($aa{m},"n"); undef $x; is($aa{m},"n"); }
is($aa{a},"b");

sub test_err_localref () {
    like($@,qr/Can't localize through a reference/,'error');
}
$x = \$aa;
my $y = \$aa;
eval { local $$x; };      test_err_localref;
eval { local ${$x}; };    test_err_localref;
eval { local $$y; };      test_err_localref;
eval { local ${$y}; };    test_err_localref;
eval { local ${\$aa}; };  test_err_localref;
eval { local ${\'aa'}; }; test_err_localref;
$x = \@aa;
$y = \@aa;
eval { local @$x; };      test_err_localref;
eval { local @{$x}; };    test_err_localref;
eval { local @$y; };      test_err_localref;
eval { local @{$y}; };    test_err_localref;
eval { local @{\@aa}; };  test_err_localref;
eval { local @{[]}; };    test_err_localref;
$x = \%aa;
$y = \%aa;
eval { local %$x; };      test_err_localref;
eval { local %{$x}; };    test_err_localref;
eval { local %$y; };      test_err_localref;
eval { local %{$y}; };    test_err_localref;
eval { local %{\%aa}; };  test_err_localref;
eval { local %{{a=>1}}; };test_err_localref;


{
    # [perl #27638] when restoring a localized variable, the thing being
    # freed shouldn't be visible
    my $ok;
    $x = 0;
    sub X::DESTROY { $ok = !ref($x); }
    {
	local $x = \ bless {}, 'X';
	1;
    }
ok($ok,'old value not visible during restore');
}
