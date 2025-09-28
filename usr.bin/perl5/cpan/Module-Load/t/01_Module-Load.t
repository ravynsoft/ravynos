### Module::Load test suite ###
use strict;
use warnings;

BEGIN {
    if( $ENV{PERL_CORE} ) {
        chdir '../lib/Module/Load' if -d '../lib/Module/Load';
        unshift @INC, '../../..';
    }
}

BEGIN { chdir 't' if -d 't' }

use lib qw[../lib to_load];
use Module::Load;
use Test::More 'no_plan';

### test loading files & modules
{   my @Map = (
        # module               flag diagnostic
        [q|Must::Be::Loaded|,   1,  'module'],
        [q|::Must::Be::Loaded|, 1,  'module'],
        [q|LoadMe.pl|,          0,  'file'  ],
        [q|LoadIt|,             1,  'ambiguous module'  ],
        [q|ToBeLoaded|,         0,  'ambiguous file'    ],
    );

    for my $aref (@Map) {
        my($mod, $flag, $diag) = @$aref;

        my $file = Module::Load::_to_file($mod, $flag);

        eval { load $mod };

        is( $@, '',                 qq[Loading $diag '$mod' $@] );
        ok( defined($INC{$file}),   qq[  '$file' found in \%INC] );
    }
}

### Test importing functions ###
{   my $mod     = 'TestModule';
    my @funcs   = qw[func1 func2];

    eval { load $mod, @funcs };
    is( $@, '', qq[Loaded exporter module '$mod'] );

    ### test if import gets called properly
    ok( $mod->imported,                 "   ->import() was called" );

    ### test if functions get exported
    for my $func (@funcs) {
        ok( $mod->can($func),           "   $mod->can( $func )" );
        ok( __PACKAGE__->can($func),    "   we ->can ( $func )" );
    }
}
