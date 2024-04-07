
use strict;
use warnings;
use Test::More;
BEGIN { plan tests => 2 };

BEGIN { $ENV{PERL_JSON_BACKEND} = 0; }

use JSON::PP;

# from https://rt.cpan.org/Ticket/Display.html?id=25162

SKIP: {
    eval {require Tie::IxHash};
    skip "Can't load Tie::IxHash.", 2 if ($@);

    my %columns;
    tie %columns, 'Tie::IxHash';

    %columns = (
    id => 'int',
    1 => 'a',
    2 => 'b',
    3 => 'c',
    4 => 'd',
    5 => 'e',
    );

    my $json = JSON::PP->new;

    my $js = $json->encode(\%columns);
    is( $js, q/{"id":"int","1":"a","2":"b","3":"c","4":"d","5":"e"}/ );

    $js = $json->pretty->encode(\%columns);
    is( $js, <<'STR' );
{
   "id" : "int",
   "1" : "a",
   "2" : "b",
   "3" : "c",
   "4" : "d",
   "5" : "e"
}
STR

}

