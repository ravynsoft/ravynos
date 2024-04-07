BEGIN {
    if ($ENV{PERL_CORE}) {
        chdir 't' if -d 't';
        @INC = ("../lib", "lib/compress");
    }
}

use lib qw(t t/compress);
use strict ;
use warnings ;

use Test::More ;

BEGIN
{
    # use Test::NoWarnings, if available
    my $extra = 0 ;
    $extra = 1
        if eval { require Test::NoWarnings ;  import Test::NoWarnings; 1 };


    my $VERSION = '2.204';
    my @NAMES = qw(

			);

    my @OPT = qw(

			);

    plan tests => 1 + @NAMES + @OPT + $extra ;

    ok 1;

    foreach my $name (@NAMES)
    {
        use_ok($name, $VERSION);
    }


    foreach my $name (@OPT)
    {
        eval " require $name " ;
        if ($@)
        {
            ok 1, "$name not available"
        }
        else
        {
            my $ver = eval("\$${name}::VERSION");
            is $ver, $VERSION, "$name version should be $VERSION"
                or diag "$name version is $ver, need $VERSION" ;
        }
    }

}
