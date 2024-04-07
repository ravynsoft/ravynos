#!./perl

use strict;
use warnings;
BEGIN {
    unless (-d 'blib') {
        chdir 't' if -d 't';
    }
    require q(./test.pl);
    set_up_inc('../lib');
}

use utf8;
use open qw( :utf8 :std );

plan(tests => 7);

{
    package 밧e텟ʇ;
    use strict;
    use warnings;
    use mro 'c3';
    
    package Ov에rꪩࡃᛝＴeŝṱ;
    use strict;
    use warnings;
    use mro 'c3';
    use base '밧e텟ʇ';        
    use overload '""' => sub { ref(shift) . " stringified" },
                 fallback => 1;
    
    sub ネᚹ { bless {} => shift }    
    
    package 읺ҎꀀḮṆᵷꜰ롬ᵕveŔŁoad엗텟ᵵ;
    use strict;
    use warnings;
    use base 'Ov에rꪩࡃᛝＴeŝṱ';
    use mro 'c3';
}

my $x = 읺ҎꀀḮṆᵷꜰ롬ᵕveŔŁoad엗텟ᵵ->ネᚹ();
object_ok($x, '읺ҎꀀḮṆᵷꜰ롬ᵕveŔŁoad엗텟ᵵ');

my $y = Ov에rꪩࡃᛝＴeŝṱ->ネᚹ();
object_ok($y, 'Ov에rꪩࡃᛝＴeŝṱ');

is("$x", '읺ҎꀀḮṆᵷꜰ롬ᵕveŔŁoad엗텟ᵵ stringified', '... got the right value when stringifing');
is("$y", 'Ov에rꪩࡃᛝＴeŝṱ stringified', '... got the right value when stringifing');

ok(($y eq 'Ov에rꪩࡃᛝＴeŝṱ stringified'), '... eq was handled correctly');

my $result;
eval { 
    $result = $x eq '읺ҎꀀḮṆᵷꜰ롬ᵕveŔŁoad엗텟ᵵ stringified' 
};
ok(!$@, '... this should not throw an exception');
ok($result, '... and we should get the true value');

