use warnings;
use strict;
use Test::More tests => 78;

use XS::APItest;

{
    local $TODO = "[perl #78502] function pointers don't match on cygwin"
        if $^O eq "cygwin";
    ok( eval { XS::APItest::test_cv_getset_call_checker(); 1 },
        "test_cv_getset_call_checker() works as expected")
        or diag $@;
}

my @z = ();
my @a = qw(a);
my @b = qw(a b);
my @c = qw(a b c);

my($foo_got, $foo_ret);
sub foo($@) { $foo_got = [ @_ ]; return "z"; }

sub bar (\@$) { }
sub baz { }

$foo_got = undef;
eval q{$foo_ret = foo(@b, @c);};
is $@, "";
is_deeply $foo_got, [ 2, qw(a b c) ];
is $foo_ret, "z";

$foo_got = undef;
eval q{$foo_ret = &foo(@b, @c);};
is $@, "";
is_deeply $foo_got, [ qw(a b), qw(a b c) ];
is $foo_ret, "z";

cv_set_call_checker_lists(\&foo);

$foo_got = undef;
eval q{$foo_ret = foo(@b, @c);};
is $@, "";
is_deeply $foo_got, [ qw(a b), qw(a b c) ];
is $foo_ret, "z";

$foo_got = undef;
eval q{$foo_ret = &foo(@b, @c);};
is $@, "";
is_deeply $foo_got, [ qw(a b), qw(a b c) ];
is $foo_ret, "z";

cv_set_call_checker_scalars(\&foo);

$foo_got = undef;
eval q{$foo_ret = foo(@b, @c);};
is $@, "";
is_deeply $foo_got, [ 2, 3 ];
is $foo_ret, "z";

$foo_got = undef;
eval q{$foo_ret = foo(@b, @c, @a, @c);};
is $@, "";
is_deeply $foo_got, [ 2, 3, 1, 3 ];
is $foo_ret, "z";

$foo_got = undef;
eval q{$foo_ret = foo(@b);};
is $@, "";
is_deeply $foo_got, [ 2 ];
is $foo_ret, "z";

$foo_got = undef;
eval q{$foo_ret = foo();};
is $@, "";
is_deeply $foo_got, [];
is $foo_ret, "z";

$foo_got = undef;
eval q{$foo_ret = &foo(@b, @c);};
is $@, "";
is_deeply $foo_got, [ qw(a b), qw(a b c) ];
is $foo_ret, "z";

cv_set_call_checker_proto(\&foo, "\\\@\$");
$foo_got = undef;
eval q{$foo_ret = foo(@b, @c);};
is $@, "";
is_deeply $foo_got, [ \@b, 3 ];
is $foo_ret, "z";

cv_set_call_checker_proto(\&foo, undef);
$foo_got = undef;
eval q{$foo_ret = foo(@b, @c);};
isnt $@, "";
is_deeply $foo_got, undef;
is $foo_ret, "z";

cv_set_call_checker_proto(\&foo, \&bar);
$foo_got = undef;
eval q{$foo_ret = foo(@b, @c);};
is $@, "";
is_deeply $foo_got, [ \@b, 3 ];
is $foo_ret, "z";

cv_set_call_checker_proto(\&foo, \&baz);
$foo_got = undef;
eval q{$foo_ret = foo(@b, @c);};
isnt $@, "";
is_deeply $foo_got, undef;
is $foo_ret, "z";

cv_set_call_checker_proto_or_list(\&foo, "\\\@\$");
$foo_got = undef;
eval q{$foo_ret = foo(@b, @c);};
is $@, "";
is_deeply $foo_got, [ \@b, 3 ];
is $foo_ret, "z";

cv_set_call_checker_proto_or_list(\&foo, undef);
$foo_got = undef;
eval q{$foo_ret = foo(@b, @c);};
is $@, "";
is_deeply $foo_got, [ qw(a b), qw(a b c) ];
is $foo_ret, "z";

cv_set_call_checker_proto_or_list(\&foo, \&bar);
$foo_got = undef;
eval q{$foo_ret = foo(@b, @c);};
is $@, "";
is_deeply $foo_got, [ \@b, 3 ];
is $foo_ret, "z";

cv_set_call_checker_proto_or_list(\&foo, \&baz);
$foo_got = undef;
eval q{$foo_ret = foo(@b, @c);};
is $@, "";
is_deeply $foo_got, [ qw(a b), qw(a b c) ];
is $foo_ret, "z";

cv_set_call_checker_multi_sum(\&foo);

$foo_got = undef;
eval q{$foo_ret = foo(@b, @c);};
is $@, "";
is_deeply $foo_got, undef;
is $foo_ret, 5;

$foo_got = undef;
eval q{$foo_ret = foo(@b);};
is $@, "";
is_deeply $foo_got, undef;
is $foo_ret, 2;

$foo_got = undef;
eval q{$foo_ret = foo();};
is $@, "";
is_deeply $foo_got, undef;
is $foo_ret, 0;

$foo_got = undef;
eval q{$foo_ret = foo(@b, @c, @a, @c);};
is $@, "";
is_deeply $foo_got, undef;
is $foo_ret, 9;

sub MODIFY_CODE_ATTRIBUTES { cv_set_call_checker_lists($_[1]); () }
BEGIN {
  *foo2 = sub($$) :Attr { $foo_got = [ @_ ]; return "z"; };
  my $foo = 3;
  *foo3 = sub() :Attr { $foo };
}

$foo_got = undef;
eval q{$foo_ret = foo2(@b, @c);};
is $@, "";
is_deeply $foo_got, [ qw(a b), qw(a b c) ];
is $foo_ret, "z";

eval q{$foo_ret = foo3(@b, @c);};
is $@, "";
is $foo_ret, 3;

cv_set_call_checker_lists(\&foo);
undef &foo;
$foo_got = undef;
eval 'sub foo($@) { $foo_got = [ @_ ]; return "z"; }
      $foo_ret = foo(@b, @c);';
is $@, "";
is_deeply $foo_got, [ 2, qw(a b c) ], 'undef clears call checkers';
is $foo_ret, "z";

my %got;

sub g {
    my $name = shift;
    my $sub = sub ($\@) {
	$got{$name} = [ @_ ];
	return $name;
    };
    cv_set_call_checker_scalars($sub);
    return $sub;
}

BEGIN {
    *whack = g("whack");
    *glurp = g("glurp");
}

%got = ();
my $whack_ret = whack(@b, @c);
is $@, "";
is_deeply $got{whack}, [ 2, 3 ];
is $whack_ret, "whack";

my $glurp_ret = glurp(@b, @c);
is $@, "";
is_deeply $got{glurp}, [ 2, 3 ];
is $glurp_ret, "glurp";

1;
