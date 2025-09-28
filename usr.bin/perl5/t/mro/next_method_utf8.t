#!/usr/bin/perl

use strict;
use warnings;
use utf8;
use open qw( :utf8 :std );

require q(./test.pl); plan(tests => 5);

=pod

This tests the classic diamond inheritance pattern.

   <A>
  /   \
<B>   <C>
  \   /
   <D>

=cut

{
    package Diӑmond_A;
    use mro 'c3'; 
    sub 헬ฬ { 'Diӑmond_A::헬ฬ' }
    sub fಓ { 'Diӑmond_A::fಓ' }       
}
{
    package Diӑmond_B;
    use base 'Diӑmond_A';
    use mro 'c3';     
    sub fಓ { 'Diӑmond_B::fಓ => ' . (shift)->next::method() }       
}
{
    package Diӑmond_C;
    use mro 'c3';    
    use base 'Diӑmond_A';     

    sub 헬ฬ { 'Diӑmond_C::헬ฬ => ' . (shift)->next::method() }
    sub fಓ { 'Diӑmond_C::fಓ => ' . (shift)->next::method() }   
}
{
    package Diӑmond_D;
    use base ('Diӑmond_B', 'Diӑmond_C');
    use mro 'c3'; 
    
    sub fಓ { 'Diӑmond_D::fಓ => ' . (shift)->next::method() }   
}

ok(eq_array(
    mro::get_linear_isa('Diӑmond_D'),
    [ qw(Diӑmond_D Diӑmond_B Diӑmond_C Diӑmond_A) ]
), '... got the right MRO for Diӑmond_D');

is(Diӑmond_D->헬ฬ, 'Diӑmond_C::헬ฬ => Diӑmond_A::헬ฬ', '... method resolved itself as expected');

is(Diӑmond_D->can('헬ฬ')->('Diӑmond_D'), 
   'Diӑmond_C::헬ฬ => Diӑmond_A::헬ฬ', 
   '... can(method) resolved itself as expected');
   
is(UNIVERSAL::can("Diӑmond_D", '헬ฬ')->('Diӑmond_D'), 
   'Diӑmond_C::헬ฬ => Diӑmond_A::헬ฬ', 
   '... can(method) resolved itself as expected');

is(Diӑmond_D->fಓ, 
    'Diӑmond_D::fಓ => Diӑmond_B::fಓ => Diӑmond_C::fಓ => Diӑmond_A::fಓ', 
    '... method fಓ resolved itself as expected');
