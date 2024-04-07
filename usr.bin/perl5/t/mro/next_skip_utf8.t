#!/usr/bin/perl

use strict;
use warnings;

require q(./test.pl); plan(tests => 10);

use utf8;
use open qw( :utf8 :std );

=pod

This tests the classic diamond inheritance pattern.

   <A>
  /   \
<B>   <C>
  \   /
   <D>

=cut

{
    package Ｄiᚪၚd_A;
    use mro 'c3'; 
    sub ᴮaȐ { 'Ｄiᚪၚd_A::ᴮaȐ' }        
    sub 바ź { 'Ｄiᚪၚd_A::바ź' }
}
{
    package Ｄiᚪၚd_B;
    use base 'Ｄiᚪၚd_A';
    use mro 'c3';    
    sub 바ź { 'Ｄiᚪၚd_B::바ź => ' . (shift)->next::method() }         
}
{
    package Ｄiᚪၚd_C;
    use mro 'c3';    
    use base 'Ｄiᚪၚd_A';     
    sub ᕘ { 'Ｄiᚪၚd_C::ᕘ' }   
    sub buƵ { 'Ｄiᚪၚd_C::buƵ' }     
    
    sub woｚ { 'Ｄiᚪၚd_C::woｚ' }
    sub maᐇbʚ { 'Ｄiᚪၚd_C::maᐇbʚ' }         
}
{
    package Ｄiᚪၚd_D;
    use base ('Ｄiᚪၚd_B', 'Ｄiᚪၚd_C');
    use mro 'c3'; 
    sub ᕘ { 'Ｄiᚪၚd_D::ᕘ => ' . (shift)->next::method() } 
    sub ᴮaȐ { 'Ｄiᚪၚd_D::ᴮaȐ => ' . (shift)->next::method() }   
    sub buƵ { 'Ｄiᚪၚd_D::buƵ => ' . (shift)->바ź() }  
    sub fuz { 'Ｄiᚪၚd_D::fuz => ' . (shift)->next::method() }  
    
    sub woｚ { 'Ｄiᚪၚd_D::woｚ can => ' . ((shift)->next::can() ? 1 : 0) }
    sub noz { 'Ｄiᚪၚd_D::noz can => ' . ((shift)->next::can() ? 1 : 0) }

    sub maᐇbʚ { 'Ｄiᚪၚd_D::maᐇbʚ => ' . ((shift)->maybe::next::method() || 0) }
    sub ᒧyベ { 'Ｄiᚪၚd_D::ᒧyベ => ' .    ((shift)->maybe::next::method() || 0) }

}

ok(eq_array(
    mro::get_linear_isa('Ｄiᚪၚd_D'),
    [ qw(Ｄiᚪၚd_D Ｄiᚪၚd_B Ｄiᚪၚd_C Ｄiᚪၚd_A) ]
), '... got the right MRO for Ｄiᚪၚd_D');

is(Ｄiᚪၚd_D->ᕘ, 'Ｄiᚪၚd_D::ᕘ => Ｄiᚪၚd_C::ᕘ', '... skipped B and went to C correctly');
is(Ｄiᚪၚd_D->ᴮaȐ, 'Ｄiᚪၚd_D::ᴮaȐ => Ｄiᚪၚd_A::ᴮaȐ', '... skipped B & C and went to A correctly');
is(Ｄiᚪၚd_D->바ź, 'Ｄiᚪၚd_B::바ź => Ｄiᚪၚd_A::바ź', '... called B method, skipped C and went to A correctly');
is(Ｄiᚪၚd_D->buƵ, 'Ｄiᚪၚd_D::buƵ => Ｄiᚪၚd_B::바ź => Ｄiᚪၚd_A::바ź', '... called D method dispatched to , different method correctly');
eval { Ｄiᚪၚd_D->fuz };
like($@, qr/^No next::method 'fuz' found for Ｄiᚪၚd_D/u, '... cannot re-dispatch to a method which is not there');
is(Ｄiᚪၚd_D->woｚ, 'Ｄiᚪၚd_D::woｚ can => 1', '... can re-dispatch figured out correctly');
is(Ｄiᚪၚd_D->noz, 'Ｄiᚪၚd_D::noz can => 0', '... cannot re-dispatch figured out correctly');

is(Ｄiᚪၚd_D->maᐇbʚ, 'Ｄiᚪၚd_D::maᐇbʚ => Ｄiᚪၚd_C::maᐇbʚ', '... redispatched D to C when it exists');
is(Ｄiᚪၚd_D->ᒧyベ, 'Ｄiᚪၚd_D::ᒧyベ => 0', '... quietly failed redispatch from D');
