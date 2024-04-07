#!./perl

use strict;
use warnings;
use utf8;
use open qw( :utf8 :std );

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
    package 텟ţ::ᴼ;
    use mro 'dfs';
    
    sub ᴼ_or_Ḋ { '텟ţ::ᴼ' }
    sub ᴼ_or_Ḟ { '텟ţ::ᴼ' }    
    
    package 텟ţ::Ḟ;
    use base '텟ţ::ᴼ';
    use mro 'dfs';
    
    sub ᴼ_or_Ḟ { '텟ţ::Ḟ' }    
    
    package 텟ţ::ऍ;
    use base '텟ţ::ᴼ';
    use mro 'dfs';
        
    package 텟ţ::Ḋ;
    use base '텟ţ::ᴼ';    
    use mro 'dfs';
    
    sub ᴼ_or_Ḋ { '텟ţ::Ḋ' }
    sub ƈ_or_Ḋ { '텟ţ::Ḋ' }
        
    package 텟ţ::ƈ;
    use base ('텟ţ::Ḋ', '텟ţ::Ḟ');
    use mro 'dfs';    

    sub ƈ_or_Ḋ { '텟ţ::ƈ' }
    
    package 텟ţ::ᛒ;
    use base ('텟ţ::ऍ', '텟ţ::Ḋ');
    use mro 'dfs';
        
    package 텟ţ::ଅ;
    use base ('텟ţ::ᛒ', '텟ţ::ƈ');
    use mro 'dfs';
}

ok(eq_array(
    mro::get_linear_isa('텟ţ::ଅ'),
    [ qw(텟ţ::ଅ 텟ţ::ᛒ 텟ţ::ऍ 텟ţ::ᴼ 텟ţ::Ḋ 텟ţ::ƈ 텟ţ::Ḟ) ]
), '... got the right MRO for 텟ţ::ଅ');      
    
is(텟ţ::ଅ->ᴼ_or_Ḋ, '텟ţ::ᴼ', '... got the right method dispatch');    
is(텟ţ::ଅ->ᴼ_or_Ḟ, '텟ţ::ᴼ', '... got the right method dispatch');   

# NOTE: 
# this test is particularly interesting because the p5 dispatch
# would actually call 텟ţ::Ḋ before 텟ţ::ƈ and 텟ţ::Ḋ is a
# subclass of 텟ţ::ƈ 
is(텟ţ::ଅ->ƈ_or_Ḋ, '텟ţ::Ḋ', '... got the right method dispatch');    
