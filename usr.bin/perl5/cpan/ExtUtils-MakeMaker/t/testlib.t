#!/usr/bin/perl -Tw

BEGIN {
    # ./lib is there so t/lib can be seen even after we chdir.
    unshift @INC, 't/lib', './lib';
}
chdir 't';

use strict;
use warnings;
use Test::More tests => 5;

BEGIN {
    # non-core tests will have blib in their path.  We remove it
    # and just use the one in lib/.
    unless( $ENV{PERL_CORE} ) {
        @INC = grep !/blib/, @INC;
        unshift @INC, '../lib';
    }
}

my @blib_paths = grep /blib/, @INC;
is( @blib_paths, 0, 'No blib dirs yet in @INC' );

use_ok( 'ExtUtils::testlib' );

@blib_paths = grep { /blib/ } @INC;
is( @blib_paths, 2, 'ExtUtils::testlib added two @INC dirs!' );
ok( !(grep !File::Spec->file_name_is_absolute($_), @blib_paths),
                    '  and theyre absolute');

eval { eval "# @INC"; };
is( $@, '',     '@INC is not tainted' );
