#!./perl

use strict;
use warnings;

require q(./test.pl); plan(tests => 4);

=pod

This example is take from: http://www.python.org/2.3/mro.html

"My second example"
class O: pass
class F(O): pass
class E(O): pass
class D(O): pass
class C(D,F): pass
class B(E,D): pass
class A(B,C): pass

                           6
                          ---
Level 3                  | O |
                       /  ---  \
                      /    |    \
                     /     |     \
                    /      |      \
                  ---     ---    ---
Level 2        2 | E | 4 | D |  | F | 5
                  ---     ---    ---
                   \      / \     /
                    \    /   \   /
                     \  /     \ /
                      ---     ---
Level 1            1 | B |   | C | 3
                      ---     ---
                       \       /
                        \     /
                          ---
Level 0                0 | A |
                          ---

>>> A.mro()
(<class '__main__.A'>, <class '__main__.B'>, <class '__main__.E'>,
<class '__main__.C'>, <class '__main__.D'>, <class '__main__.F'>,
<type 'object'>)

=cut

{
    package Test::O;
    use mro 'c3';
    
    sub O_or_D { 'Test::O' }
    sub O_or_F { 'Test::O' }    
    
    package Test::F;
    use base 'Test::O';
    use mro 'c3';
    
    sub O_or_F { 'Test::F' }    
    
    package Test::E;
    use base 'Test::O';
    use mro 'c3';
        
    package Test::D;
    use base 'Test::O';    
    use mro 'c3';
    
    sub O_or_D { 'Test::D' }
    sub C_or_D { 'Test::D' }
        
    package Test::C;
    use base ('Test::D', 'Test::F');
    use mro 'c3';    

    sub C_or_D { 'Test::C' }
    
    package Test::B;
    use base ('Test::E', 'Test::D');
    use mro 'c3';
        
    package Test::A;
    use base ('Test::B', 'Test::C');
    use mro 'c3';
}

ok(eq_array(
    mro::get_linear_isa('Test::A'),
    [ qw(Test::A Test::B Test::E Test::C Test::D Test::F Test::O) ]
), '... got the right MRO for Test::A');      
    
is(Test::A->O_or_D, 'Test::D', '... got the right method dispatch');    
is(Test::A->O_or_F, 'Test::F', '... got the right method dispatch');   

# NOTE: 
# this test is particularly interesting because the p5 dispatch
# would actually call Test::D before Test::C and Test::D is a
# subclass of Test::C 
is(Test::A->C_or_D, 'Test::C', '... got the right method dispatch');    
