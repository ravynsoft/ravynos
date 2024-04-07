#!./perl

use strict;
use warnings;

require q(./test.pl); plan(tests => 1);

=pod

example taken from: L<http://www.opendylan.org/books/drm/Method_Dispatch>

         Object
           ^
           |
        LifeForm 
         ^    ^
        /      \
   Sentient    BiPedal
      ^          ^
      |          |
 Intelligent  Humanoid
       ^        ^
        \      /
         Vulcan

 define class <sentient> (<life-form>) end class;
 define class <bipedal> (<life-form>) end class;
 define class <intelligent> (<sentient>) end class;
 define class <humanoid> (<bipedal>) end class;
 define class <vulcan> (<intelligent>, <humanoid>) end class;

=cut

{
    package Object;    
    use mro 'dfs';
    
    package LifeForm;
    use mro 'dfs';
    use base 'Object';
    
    package Sentient;
    use mro 'dfs';
    use base 'LifeForm';
    
    package BiPedal;
    use mro 'dfs';    
    use base 'LifeForm';
    
    package Intelligent;
    use mro 'dfs';    
    use base 'Sentient';
    
    package Humanoid;
    use mro 'dfs';    
    use base 'BiPedal';
    
    package Vulcan;
    use mro 'dfs';    
    use base ('Intelligent', 'Humanoid');
}

ok(eq_array(
    mro::get_linear_isa('Vulcan'),
    [ qw(Vulcan Intelligent Sentient LifeForm Object Humanoid BiPedal) ]
), '... got the right MRO for the Vulcan Dylan Example');  
