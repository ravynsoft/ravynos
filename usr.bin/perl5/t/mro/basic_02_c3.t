#!./perl

use strict;
use warnings;

require q(./test.pl); plan(tests => 10);

=pod

This example is take from: http://www.python.org/2.3/mro.html

"My first example"
class O: pass
class F(O): pass
class E(O): pass
class D(O): pass
class C(D,F): pass
class B(D,E): pass
class A(B,C): pass


                          6
                         ---
Level 3                 | O |                  (more general)
                      /  ---  \
                     /    |    \                      |
                    /     |     \                     |
                   /      |      \                    |
                  ---    ---    ---                   |
Level 2        3 | D | 4| E |  | F | 5                |
                  ---    ---    ---                   |
                   \  \ _ /       |                   |
                    \    / \ _    |                   |
                     \  /      \  |                   |
                      ---      ---                    |
Level 1            1 | B |    | C | 2                 |
                      ---      ---                    |
                        \      /                      |
                         \    /                      \ /
                           ---
Level 0                 0 | A |                (more specialized)
                           ---

=cut

{
    package Test::O;
    use mro 'c3'; 
    
    package Test::F;   
    use mro 'c3';  
    use base 'Test::O';        
    
    package Test::E;
    use base 'Test::O';    
    use mro 'c3';     
    
    sub C_or_E { 'Test::E' }

    package Test::D;
    use mro 'c3'; 
    use base 'Test::O';     
    
    sub C_or_D { 'Test::D' }       
      
    package Test::C;
    use base ('Test::D', 'Test::F');
    use mro 'c3'; 
    
    sub C_or_D { 'Test::C' }
    sub C_or_E { 'Test::C' }    
        
    package Test::B;    
    use mro 'c3'; 
    use base ('Test::D', 'Test::E');    
        
    package Test::A;    
    use base ('Test::B', 'Test::C');
    use mro 'c3';    
}

ok(eq_array(
    mro::get_linear_isa('Test::F'),
    [ qw(Test::F Test::O) ]
), '... got the right MRO for Test::F');

ok(eq_array(
    mro::get_linear_isa('Test::E'),
    [ qw(Test::E Test::O) ]
), '... got the right MRO for Test::E');    

ok(eq_array(
    mro::get_linear_isa('Test::D'),
    [ qw(Test::D Test::O) ]
), '... got the right MRO for Test::D');       

ok(eq_array(
    mro::get_linear_isa('Test::C'),
    [ qw(Test::C Test::D Test::F Test::O) ]
), '... got the right MRO for Test::C'); 

ok(eq_array(
    mro::get_linear_isa('Test::B'),
    [ qw(Test::B Test::D Test::E Test::O) ]
), '... got the right MRO for Test::B');     

ok(eq_array(
    mro::get_linear_isa('Test::A'),
    [ qw(Test::A Test::B Test::C Test::D Test::E Test::F Test::O) ]
), '... got the right MRO for Test::A');  
    
is(Test::A->C_or_D, 'Test::C', '... got the expected method output');
is(Test::A->can('C_or_D')->(), 'Test::C', '... can got the expected method output');
is(Test::A->C_or_E, 'Test::C', '... got the expected method output');
is(Test::A->can('C_or_E')->(), 'Test::C', '... can got the expected method output');
