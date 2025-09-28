#!./perl

use strict;
use warnings;

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
    package Diamond_A;
    sub hello { 'Diamond_A::hello' }
}
{
    package Diamond_B;
    use base 'Diamond_A';
}
{
    package Diamond_C;
    use base 'Diamond_A';     
    
    sub hello { 'Diamond_C::hello' }
}
{
    package Diamond_D;
    use base ('Diamond_B', 'Diamond_C');
    use mro 'dfs';
}

ok(eq_array(
    mro::get_linear_isa('Diamond_D'),
    [ qw(Diamond_D Diamond_B Diamond_A Diamond_C) ]
), '... got the right MRO for Diamond_D');

is(Diamond_D->hello, 'Diamond_A::hello', '... method resolved itself as expected');
is(Diamond_D->can('hello')->(), 'Diamond_A::hello', '... can(method) resolved itself as expected');
is(UNIVERSAL::can("Diamond_D", 'hello')->(), 'Diamond_A::hello', '... can(method) resolved itself as expected');
