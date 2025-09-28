#!./perl

use strict;
use warnings;
use utf8;
use open qw( :utf8 :std );

require q(./test.pl); plan(tests => 1);

=pod 

From the parrot test t/pmc/object-meths.t

 A   B A   E
  \ /   \ /
   C     D
    \   /
     \ /
      F

=cut

{
    package Ƭ::ŁiƁ::ଅ; use mro 'c3';
    package Ƭ::ŁiƁ::ᛒ; use mro 'c3';
    package Ƭ::ŁiƁ::ऍ; use mro 'c3';
    package Ƭ::ŁiƁ::ƈ; use mro 'c3'; use base ('Ƭ::ŁiƁ::ଅ', 'Ƭ::ŁiƁ::ᛒ');
    package Ƭ::ŁiƁ::Ḋ; use mro 'c3'; use base ('Ƭ::ŁiƁ::ଅ', 'Ƭ::ŁiƁ::ऍ');
    package Ƭ::ŁiƁ::Ḟ; use mro 'c3'; use base ('Ƭ::ŁiƁ::ƈ', 'Ƭ::ŁiƁ::Ḋ');
}

ok(eq_array(
    mro::get_linear_isa('Ƭ::ŁiƁ::Ḟ'),
    [ qw(Ƭ::ŁiƁ::Ḟ Ƭ::ŁiƁ::ƈ Ƭ::ŁiƁ::Ḋ Ƭ::ŁiƁ::ଅ Ƭ::ŁiƁ::ᛒ Ƭ::ŁiƁ::ऍ) ]
), '... got the right MRO for Ƭ::ŁiƁ::Ḟ');  

