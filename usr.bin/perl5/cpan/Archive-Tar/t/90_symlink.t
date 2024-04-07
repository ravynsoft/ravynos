BEGIN { chdir 't' if -d 't' }

use lib '../lib';

use strict;
use File::Spec;
use File::Path;
use Test::More;

### developer tests mostly, so enable them with an extra argument
plan skip_all => "Skipping tests on this platform" unless @ARGV;
plan 'no_plan';

my $Class   = 'Archive::Tar';
my $Dir     = File::Spec->catdir( qw[src linktest] );
my %Map     = (
    File::Spec->catfile( $Dir, "linktest_with_dir.tar" ) => [
        [ 0, qr/SECURE EXTRACT MODE/ ],
        [ 1, qr/^$/ ]
    ],
    File::Spec->catfile( $Dir, "linktest_missing_dir.tar" ) => [
        [ 0, qr/SECURE EXTRACT MODE/ ],
        [ 0, qr/File exists/ ],
    ],
);

use_ok( $Class );

{   while( my($file, $aref) = each %Map ) {

        for my $mode ( 0, 1 ) {
            my $expect = $aref->[$mode]->[0];
            my $regex  = $aref->[$mode]->[1];

            my $tar  = $Class->new( $file );
            ok( $tar,                   "Object created from $file" );

            ### damn warnings
            local $Archive::Tar::INSECURE_EXTRACT_MODE = $mode;
            local $Archive::Tar::INSECURE_EXTRACT_MODE = $mode;

            ok( 1,                  "   Extracting with insecure mode: $mode" );

            my $warning;
            local $SIG{__WARN__} = sub { $warning .= "@_"; warn @_; };

            my $rv = eval { $tar->extract } || 0;
            ok( !$@,                "       No fatal error" );
            is( !!$rv, !!$expect,   "       RV as expected" );
            like( $warning, $regex, "       Error matches $regex" );

            rmtree( 'linktest' );
        }
    }
}
