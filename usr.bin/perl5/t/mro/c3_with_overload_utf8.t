#!/usr/bin/perl

use strict;
use warnings;

require q(./test.pl); plan(tests => 7);

{
    package BaseTest;
    use strict;
    use warnings;
    use mro 'c3';
    
    package OverloadingTest;
    use strict;
    use warnings;
    use mro 'c3';
    use base 'BaseTest';        
    use overload '""' => sub { ref(shift) . " stringified" },
                 fallback => 1;
    
    sub new { bless {} => shift }    
    
    package InheritingFromOverloadedTest;
    use strict;
    use warnings;
    use base 'OverloadingTest';
    use mro 'c3';
}

my $x = InheritingFromOverloadedTest->new();
object_ok($x, 'InheritingFromOverloadedTest');

my $y = OverloadingTest->new();
object_ok($y, 'OverloadingTest');

is("$x", 'InheritingFromOverloadedTest stringified', '... got the right value when stringifing');
is("$y", 'OverloadingTest stringified', '... got the right value when stringifing');

ok(($y eq 'OverloadingTest stringified'), '... eq was handled correctly');

my $result;
eval { 
    $result = $x eq 'InheritingFromOverloadedTest stringified' 
};
ok(!$@, '... this should not throw an exception');
ok($result, '... and we should get the true value');
