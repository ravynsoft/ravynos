use strict;
use warnings;
use Test::More tests => 2;

BEGIN {
    package MyTie;
    BEGIN { $INC{'MyTie.pm'} = 1 }

    use Attribute::Handlers autotie => { '__CALLER__::Mine' => __PACKAGE__ };

    sub TIESCALAR {
        my ($class, $data) = @_;
        bless \$data, $class;
    }

    sub FETCH { ${$_[0]} }
    sub STORE { ${$_[0]} = $_[1] }
}

use MyTie;

eval q{
    my $var :Mine;
    1;
};
::is $@, '',
    'attribute available in caller';

{
    package Pack2;
    use MyTie;

    eval q{
        my $var :Mine;
        1;
    };
    ::is $@, '',
        'attribute available in caller of second package';
}
