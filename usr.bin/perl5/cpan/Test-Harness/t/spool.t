#!/usr/bin/perl -w

BEGIN {
    unshift @INC, 't/lib';
}

# test T::H::_open_spool and _close_spool - these are good examples
# of the 'Fragile Test' pattern - messing with I/O primitives breaks
# nearly everything

use strict;
use warnings;
use Test::More;

my $useOrigOpen;
my $useOrigClose;

# setup replacements for core open and close - breaking these makes everything very fragile
BEGIN {
    $useOrigOpen = $useOrigClose = 1;

    # taken from http://www.perl.com/pub/a/2002/06/11/threads.html?page=2

    *CORE::GLOBAL::open = \&my_open;

    sub my_open (*@) {
        if ($useOrigOpen) {
            if ( defined( $_[0] ) ) {
                use Symbol qw();
                my $handle = Symbol::qualify( $_[0], (caller)[0] );
                no strict 'refs';
                if ( @_ == 1 ) {
                    return CORE::open($handle);
                }
                elsif ( @_ == 2 ) {
                    return CORE::open( $handle, $_[1] );
                }
                else {
                    die "Can't open with more than two args";
                }
            }
        }
        else {
            return;
        }
    }

    *CORE::GLOBAL::close = sub (*) {
        if   ($useOrigClose) { return CORE::close(shift) }
        else                 {return}
    };

}

use TAP::Harness;
use TAP::Parser;
use TAP::Parser::Iterator::Array;

plan tests => 4;

{

    # coverage tests for the basically untested T::H::_open_spool

    my @spool = ( 't', 'spool' );
    $ENV{PERL_TEST_HARNESS_DUMP_TAP} = File::Spec->catfile(@spool);

# now given that we're going to be writing stuff to the file system, make sure we have
# a cleanup hook

    END {
        use File::Path;

        $useOrigOpen = $useOrigClose = 1;

        # remove the tree if we made it this far
        rmtree( $ENV{PERL_TEST_HARNESS_DUMP_TAP} )
          if $ENV{PERL_TEST_HARNESS_DUMP_TAP};
    }

    my @die;

    eval {
        local $SIG{__DIE__} = sub { push @die, @_ };

        # use the broken open
        $useOrigOpen = 0;

        TAP::Harness->_open_spool(
            File::Spec->catfile(qw (source_tests harness )) );

        # restore universal sanity
        $useOrigOpen = 1;
    };

    is @die, 1, 'open failed, die as expected';

    my $spoolDir = quotemeta(
        File::Spec->catfile( @spool, qw( source_tests harness ) ) );

    like pop @die, qr/ Can't write $spoolDir \( /, '...with expected message';

    # now make close fail

    use Symbol;

    my $spoolHandle = gensym;

    my $tap = <<'END_TAP';
1..1
ok 1 - input file opened

END_TAP

    my $parser = TAP::Parser->new(
        {   spool => $spoolHandle,
            iterator =>
              TAP::Parser::Iterator::Array->new( [ split /\n/ => $tap ] )
        }
    );

    @die = ();

    eval {
        local $SIG{__DIE__} = sub { push @die, @_ };

        # use the broken CORE::close
        $useOrigClose = 0;

        TAP::Harness->_close_spool($parser);

        $useOrigClose = 1;
    };

    unless ( is @die, 1, 'close failed, die as expected' ) {
        diag " >>> $_ <<<\n" for @die;
    }

    like pop @die, qr/ Error closing TAP spool file[(] /,
      '...with expected message';
}
