#!/usr/bin/perl

use strict;
use warnings;
use NEXT;
use utf8;
use open qw( :utf8 :std );

chdir 't' if -d 't';
require './test.pl';
plan(tests => 4);

{
    package ᕘ;
    use strict;
    use warnings;
    use mro 'c3';
    
    sub fಓ { 'ᕘ::fಓ' }
    
    package Fᶽ;
    use strict;
    use warnings;
    use mro 'c3';
    use base 'ᕘ';

    sub fಓ { 'Fᶽ::fಓ => ' . (shift)->next::method }
        
    package Bᛆ;
    use strict;
    use warnings;    
    use mro 'c3';
    use base 'ᕘ';

    sub fಓ { 'Bᛆ::fಓ => ' . (shift)->next::method }
    
    package Baᕃ;
    use strict;
    use warnings;    

    use base 'Bᛆ', 'Fᶽ';
    
    sub fಓ { 'Baᕃ::fಓ => ' . (shift)->NEXT::fಓ }    
}

is(ᕘ->fಓ, 'ᕘ::fಓ', '... got the right value from ᕘ->fಓ');
is(Fᶽ->fಓ, 'Fᶽ::fಓ => ᕘ::fಓ', '... got the right value from Fᶽ->fಓ');
is(Bᛆ->fಓ, 'Bᛆ::fಓ => ᕘ::fಓ', '... got the right value from Bᛆ->fಓ');

is(Baᕃ->fಓ, 'Baᕃ::fಓ => Bᛆ::fಓ => Fᶽ::fಓ => ᕘ::fಓ', '... got the right value using NEXT in a subclass of a C3 class');

