#!./perl

use strict;
use warnings;
use utf8;
use open qw( :utf8 :std );

require q(./test.pl); plan(tests => 4);

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
    sub hèllò { 'Ｄiᚪၚd_A::hèllò' }
}
{
    package Ｄiᚪၚd_B;
    use base 'Ｄiᚪၚd_A';
}
{
    package Ｄiᚪၚd_C;
    use base 'Ｄiᚪၚd_A';     
    
    sub hèllò { 'Ｄiᚪၚd_C::hèllò' }
}
{
    package Ｄiᚪၚd_D;
    use base ('Ｄiᚪၚd_B', 'Ｄiᚪၚd_C');
    use mro 'c3';
}

ok(eq_array(
    mro::get_linear_isa('Ｄiᚪၚd_D'),
    [ qw(Ｄiᚪၚd_D Ｄiᚪၚd_B Ｄiᚪၚd_C Ｄiᚪၚd_A) ]
), '... got the right MRO for Ｄiᚪၚd_D');

is(Ｄiᚪၚd_D->hèllò, 'Ｄiᚪၚd_C::hèllò', '... method resolved itself as expected');
is(Ｄiᚪၚd_D->can('hèllò')->(), 'Ｄiᚪၚd_C::hèllò', '... can(method) resolved itself as expected');
is(UNIVERSAL::can("Ｄiᚪၚd_D", 'hèllò')->(), 'Ｄiᚪၚd_C::hèllò', '... can(method) resolved itself as expected');
