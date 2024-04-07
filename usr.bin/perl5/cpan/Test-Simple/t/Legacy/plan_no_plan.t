BEGIN {
    if( $ENV{PERL_CORE} ) {
        chdir 't';
        @INC = '../lib';
    }
}

use Test::More;

BEGIN {
    if( !$ENV{HARNESS_ACTIVE} && $ENV{PERL_CORE} ) {
        plan skip_all => "Won't work with t/TEST";
    }
}

plan 'no_plan';

pass('Just testing');
ok(1, 'Testing again');

{
    my $warning = '';
    local $SIG{__WARN__} = sub { $warning = join "", @_ };
    SKIP: {
        skip 'Just testing skip with no_plan';
        fail("So very failed");
    }
    is( $warning, '', 'skip with no "how_many" ok with no_plan' );


    $warning = '';
    TODO: {
        todo_skip "Just testing todo_skip";

        fail("Just testing todo");
        die "todo_skip should prevent this";
        pass("Again");
    }
    is( $warning, '', 'skip with no "how_many" ok with no_plan' );
}
