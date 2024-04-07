#!./perl

BEGIN {
    chdir 't' if -d 't';
    @INC = '../lib';
    require './test.pl';
}

plan( tests => 3);
use warnings;
use strict;

package Foo;
use overload
  nomethod => sub { die "unimplemented\n" };
sub new { bless {}, shift };

package main;

my $foo = Foo->new;

eval {my $val = $foo + 1};
ok( $@ =~ /unimplemented/, "'+'  not implemented; 'nomethod' special key invoked" );

eval {$foo += 1};
ok( $@ =~ /unimplemented/, "'+=' not implemented; 'nomethod' special key invoked"  );

eval {my $val = 0; $val += $foo};
ok( $@ =~ /unimplemented/, "'+=' not implemented; 'nomethod' special key invoked"  );

