#!/usr/bin/perl

use strict;
use warnings;

require q(./test.pl); plan(tests => 2);

=pod

This tests the successful handling of a next::method call from within an
anonymous subroutine.

=cut

{
    package AA;
    use mro 'c3'; 

    sub foo {
      return 'AA::foo';
    }

    sub bar {
      return 'AA::bar';
    }
}

{
    package BB;
    use base 'AA';
    use mro 'c3'; 
    
    sub foo {
      my $code = sub {
        return 'BB::foo => ' . (shift)->next::method();
      };
      return (shift)->$code;
    }

    sub bar {
      my $code1 = sub {
        my $code2 = sub {
          return 'BB::bar => ' . (shift)->next::method();
        };
        return (shift)->$code2;
      };
      return (shift)->$code1;
    }
}

is(BB->foo, "BB::foo => AA::foo",
   'method resolved inside anonymous sub');

is(BB->bar, "BB::bar => AA::bar",
   'method resolved inside nested anonymous subs');


