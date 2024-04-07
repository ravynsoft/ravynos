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
    use mro 'c3';
    
    package LifeForm;
    use mro 'c3';
    use base 'Object';
    
    package Sentient;
    use mro 'c3';
    use base 'LifeForm';
    
    package BiPedal;
    use mro 'c3';    
    use base 'LifeForm';
    
    package Intelligent;
    use mro 'c3';    
    use base 'Sentient';
    
    package Humanoid;
    use mro 'c3';    
    use base 'BiPedal';
    
    package Vulcan;
    use mro 'c3';    
    use base ('Intelligent', 'Humanoid');
}

ok(eq_array(
    mro::get_linear_isa('Vulcan'),
    [ qw(Vulcan Intelligent Sentient Humanoid BiPedal LifeForm Object) ]
), '... got the right MRO for the Vulcan Dylan Example');  
