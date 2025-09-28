#!./perl

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc( qw(../lib) );
    plan (tests => 22); # some tests are run in BEGIN block
}

is __SUB__, "__SUB__", '__SUB__ is a bareword outside of use feature';

{
    use v5.15;
    is __SUB__, undef, '__SUB__ under use v5.16';
}

use feature 'current_sub';

is __SUB__, undef, '__SUB__ returns undef outside of a subroutine';
is +()=__SUB__, 1, '__SUB__ returns undef in list context';

sub foo { __SUB__ }
is foo, \&foo, '__SUB__ inside a named subroutine';
is foo->(), \&foo, '__SUB__ is callable';
is ref foo, 'CODE', '__SUB__ is a code reference';

my $subsub = sub { __SUB__ };
is &$subsub, $subsub, '__SUB__ inside anonymous non-closure';

my @subsubs;
for my $x(1..3) {
  push @subsubs, sub { return $x if @_; __SUB__ };
}
# Don’t loop here; we need to avoid interactions between the iterator
# and the closure.
is $subsubs[0]()(0), 1, '__SUB__ inside closure (1)';
is $subsubs[1]()(0), 2, '__SUB__ inside closure (2)';
is $subsubs[2]()(0), 3, '__SUB__ inside closure (3)';

BEGIN {
    return "begin 1" if @_;
    is CORE::__SUB__->(0), "begin 1", 'in BEGIN block'
}
BEGIN {
    return "begin 2" if @_;
    is &CORE::__SUB__->(0), "begin 2", 'in BEGIN block via & (unoptimised)'
}

sub bar;
sub bar {
    () = sort {
          is  CORE::__SUB__, \&bar,   'in sort block in sub with forw decl'
         } 1,2;
}
bar();
sub bur;
sub bur {
    () = sort {
          is &CORE::__SUB__, \&bur, '& in sort block in sub with forw decl'
         } 1,2;
}
bur();

sub squog;
sub squog {
    grep { is  CORE::__SUB__, \&squog,
          'in grep block in sub with forw decl'
    } 1;
}
squog();
sub squag;
sub squag {
    grep { is &CORE::__SUB__, \&squag,
          '& in grep block in sub with forw decl'
    } 1;
}
squag();

sub f () { __SUB__ }
is f, \&f, 'sub named () { __SUB__ } returns self ref';
my $f = sub () { __SUB__ };
is &$f, $f, 'anonymous sub(){__SUB__} returns self ref';
my $f2 = sub () { $f++ if 0; __SUB__ };
is &$f2, $f2, 'sub(){__SUB__} anonymous closure returns self ref';
$f = sub () { eval ""; __SUB__ };
is &$f, $f, 'anonymous sub(){eval ""; __SUB__} returns self ref';
{
    local $ENV{PERL5DB} = 'sub DB::DB {}';
    is runperl(
        switches => [ '-d' ],
        prog => '$f = sub(){CORE::__SUB__}; print qq-ok\n- if $f == &$f;',
       ),
      "ok\n",
      'sub(){__SUB__} under -d';
}

