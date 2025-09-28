#!/usr/bin/perl

use strict;
use warnings;

require q(./test.pl); plan(tests => 10);

=pod

This tests the classic diamond inheritance pattern.

   <A>
  /   \
<B>   <C>
  \   /
   <D>

=cut

{
    package Diamond_A;
    use mro 'c3'; 
    sub bar { 'Diamond_A::bar' }        
    sub baz { 'Diamond_A::baz' }
}
{
    package Diamond_B;
    use base 'Diamond_A';
    use mro 'c3';    
    sub baz { 'Diamond_B::baz => ' . (shift)->next::method() }         
}
{
    package Diamond_C;
    use mro 'c3';    
    use base 'Diamond_A';     
    sub foo { 'Diamond_C::foo' }   
    sub buz { 'Diamond_C::buz' }     
    
    sub woz { 'Diamond_C::woz' }
    sub maybe { 'Diamond_C::maybe' }         
}
{
    package Diamond_D;
    use base ('Diamond_B', 'Diamond_C');
    use mro 'c3'; 
    sub foo { 'Diamond_D::foo => ' . (shift)->next::method() } 
    sub bar { 'Diamond_D::bar => ' . (shift)->next::method() }   
    sub buz { 'Diamond_D::buz => ' . (shift)->baz() }  
    sub fuz { 'Diamond_D::fuz => ' . (shift)->next::method() }  
    
    sub woz { 'Diamond_D::woz can => ' . ((shift)->next::can() ? 1 : 0) }
    sub noz { 'Diamond_D::noz can => ' . ((shift)->next::can() ? 1 : 0) }

    sub maybe { 'Diamond_D::maybe => ' . ((shift)->maybe::next::method() || 0) }
    sub moybe { 'Diamond_D::moybe => ' . ((shift)->maybe::next::method() || 0) }             

}

ok(eq_array(
    mro::get_linear_isa('Diamond_D'),
    [ qw(Diamond_D Diamond_B Diamond_C Diamond_A) ]
), '... got the right MRO for Diamond_D');

is(Diamond_D->foo, 'Diamond_D::foo => Diamond_C::foo', '... skipped B and went to C correctly');
is(Diamond_D->bar, 'Diamond_D::bar => Diamond_A::bar', '... skipped B & C and went to A correctly');
is(Diamond_D->baz, 'Diamond_B::baz => Diamond_A::baz', '... called B method, skipped C and went to A correctly');
is(Diamond_D->buz, 'Diamond_D::buz => Diamond_B::baz => Diamond_A::baz', '... called D method dispatched to , different method correctly');
eval { Diamond_D->fuz };
like($@, qr/^No next::method 'fuz' found for Diamond_D/, '... cannot re-dispatch to a method which is not there');

is(Diamond_D->woz, 'Diamond_D::woz can => 1', '... can re-dispatch figured out correctly');
is(Diamond_D->noz, 'Diamond_D::noz can => 0', '... cannot re-dispatch figured out correctly');

is(Diamond_D->maybe, 'Diamond_D::maybe => Diamond_C::maybe', '... redispatched D to C when it exists');
is(Diamond_D->moybe, 'Diamond_D::moybe => 0', '... quietly failed redispatch from D');
