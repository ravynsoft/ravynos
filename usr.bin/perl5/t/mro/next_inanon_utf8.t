#!/usr/bin/perl

use strict;
use warnings;
use utf8;
use open qw( :utf8 :std );
require q(./test.pl); plan(tests => 2);

=pod

This tests the successful handling of a next::method call from within an
anonymous subroutine.

=cut

{
    package ㅏ;
    use mro 'c3'; 

    sub ᕘ {
      return 'ㅏ::ᕘ';
    }

    sub Ḃᛆ {
      return 'ㅏ::Ḃᛆ';
    }
}

{
    package Ḃ;
    use base 'ㅏ';
    use mro 'c3'; 
    
    sub ᕘ {
      my $code = sub {
        return 'Ḃ::ᕘ => ' . (shift)->next::method();
      };
      return (shift)->$code;
    }

    sub Ḃᛆ {
      my $code1 = sub {
        my $code2 = sub {
          return 'Ḃ::Ḃᛆ => ' . (shift)->next::method();
        };
        return (shift)->$code2;
      };
      return (shift)->$code1;
    }
}

is(Ḃ->ᕘ, "Ḃ::ᕘ => ㅏ::ᕘ",
   'method resolved inside anonymous sub');

is(Ḃ->Ḃᛆ, "Ḃ::Ḃᛆ => ㅏ::Ḃᛆ",
   'method resolved inside nested anonymous subs');


