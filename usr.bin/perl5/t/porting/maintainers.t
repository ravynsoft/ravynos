#!./perl -w

# Test that there are no missing Maintainers in Maintainers.pl

BEGIN {
	# This test script uses a slightly atypical invocation of the 'standard'
	# core testing setup stanza.
	# The existing porting tools which manage the Maintainers file all
	# expect to be run from the root
	# XXX that should be fixed

    chdir '..' unless -d 't';
    @INC = qw(lib Porting);
    require './t/test.pl';
}

use Config;
if ( $Config{usecrosscompile} ) {
  skip_all( "Odd failures during cross-compilation" );
}

if ( $Config{ccflags} =~ /-DPERL_EXTERNAL_GLOB/) {
    skip_all "Maintainers doesn't currently work for '-DPERL_EXTERNAL_GLOB'";
}

if ($^O eq 'VMS') {
    skip_all "home-grown glob doesn't handle fancy patterns";
}

use strict;
use warnings;
use Maintainers qw(show_results process_options finish_tap_output);

{
    local @ARGV = qw|--checkmani|;
    show_results(process_options());
}

{
    local @ARGV = qw|--checkmani lib/ ext/|;
    show_results(process_options());
}

finish_tap_output();

# EOF
