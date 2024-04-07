#!./perl

use strict;
use warnings;

require q(./test.pl); plan(tests => 2);

=pod

This tests a strange bug found by Matt S. Trout 
while building DBIx::Class. Thanks Matt!!!! 

   <A>
  /   \
<C>   <B>
  \   /
   <D>

=cut

{
    package Diamond_A;
    use mro 'c3'; 

    sub foo { 'Diamond_A::foo' }
}
{
    package Diamond_B;
    use base 'Diamond_A';
    use mro 'c3';     

    sub foo { 'Diamond_B::foo => ' . (shift)->SUPER::foo }
}
{
    package Diamond_C;
    use mro 'c3';    
    use base 'Diamond_A';     

}
{
    package Diamond_D;
    use base ('Diamond_C', 'Diamond_B');
    use mro 'c3';    
    
    sub foo { 'Diamond_D::foo => ' . (shift)->SUPER::foo }    
}

ok(eq_array(
    mro::get_linear_isa('Diamond_D'),
    [ qw(Diamond_D Diamond_C Diamond_B Diamond_A) ]
), '... got the right MRO for Diamond_D');

is(Diamond_D->foo, 
   'Diamond_D::foo => Diamond_B::foo => Diamond_A::foo', 
   '... got the right next::method dispatch path');
