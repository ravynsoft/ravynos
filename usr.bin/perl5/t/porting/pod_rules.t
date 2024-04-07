#!./perl

# What does this test?
# This test executes Porting/pod_rules.pl and reports the result of that
# program.
#
# Why do we test this?
# Among other reasons, to check the well-formed-ness of these files:
#   MANIFEST
#   win32/Makefile
#   win32/pod.mak
#   Makefile.SH
#   vms/descrip_mms.template
#
# It's broken - how do I fix it?
# If MANIFEST fails the 'up to date' test, it will probably also fail
# t/porting/manifest.t as well.  Follow instructions in that file for
# correcting the MANIFEST.  When that file passes, the MANIFEST check in this
# file will probably pass as well.  If not, it may be that the entry in
# MANIFEST is no longer in sync with the entry in perl.pod.

BEGIN {
    # We need to be at the top level dir.
    if (-f 'test.pl' && -f 'harness') {
      chdir '..';
    }
    @INC = qw(lib .); # Special @INC.
    require './t/test.pl';
}

use strict;

use Config;
if ( $Config{usecrosscompile} ) {
  skip_all( "Not all files are available during cross-compilation" );
}

my $result = runperl(switches => ['-f', '-Ilib'], 
                     progfile => 'Porting/pod_rules.pl',
                     args     => ['--tap'],
                     nolib    => 1,
                     );

print $result;
