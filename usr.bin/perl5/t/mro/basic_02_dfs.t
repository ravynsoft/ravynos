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
    use mro 'dfs'; 
    
    package Test::F;   
    use mro 'dfs';  
    use base 'Test::O';        
    
    package Test::E;
    use base 'Test::O';    
    use mro 'dfs';     
    
    sub C_or_E { 'Test::E' }

    package Test::D;
    use mro 'dfs'; 
    use base 'Test::O';     
    
    sub C_or_D { 'Test::D' }       
      
    package Test::C;
    use base ('Test::D', 'Test::F');
    use mro 'dfs'; 
    
    sub C_or_D { 'Test::C' }
    sub C_or_E { 'Test::C' }    
        
    package Test::B;    
    use mro 'dfs'; 
    use base ('Test::D', 'Test::E');    
        
    package Test::A;    
    use base ('Test::B', 'Test::C');
    use mro 'dfs';    
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
    [ qw(Test::C Test::D Test::O Test::F) ]
), '... got the right MRO for Test::C'); 

ok(eq_array(
    mro::get_linear_isa('Test::B'),
    [ qw(Test::B Test::D Test::O Test::E) ]
), '... got the right MRO for Test::B');     

ok(eq_array(
    mro::get_linear_isa('Test::A'),
    [ qw(Test::A Test::B Test::D Test::O Test::E Test::C Test::F) ]
), '... got the right MRO for Test::A');  
    
is(Test::A->C_or_D, 'Test::D', '... got the expected method output');
is(Test::A->can('C_or_D')->(), 'Test::D', '... can got the expected method output');
is(Test::A->C_or_E, 'Test::E', '... got the expected method output');
is(Test::A->can('C_or_E')->(), 'Test::E', '... can got the expected method output');
