#!./perl

package Foo;

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}

plan 9;

use constant MyClass => 'Foo::Bar::Biz::Baz';

{
    package Foo::Bar::Biz::Baz;
    1;
}

for (qw(Foo Foo:: MyClass __PACKAGE__)) {
    eval "sub { my $_ \$obj = shift; }";
    ok ! $@;
#    print $@ if $@;
}

use constant NoClass => 'Nope::Foo::Bar::Biz::Baz';

for (qw(Nope Nope:: NoClass)) {
    eval "sub { my $_ \$obj = shift; }";
    ok $@;
#    print $@ if $@;
}

is runperl(prog => 'my main $r; sub FIELDS; $$r{foo}; print qq-ok\n-'),
  "ok\n",
  'no crash with hash element when FIELDS sub stub exists';
is runperl(prog => 'my main $r; sub FIELDS; @$r{f,b}; print qq-ok\n-'),
  "ok\n",
  'no crash with hash slice when FIELDS sub stub exists';
