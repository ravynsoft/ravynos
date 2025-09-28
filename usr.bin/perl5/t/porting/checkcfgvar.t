#!./perl -w

# What does this test?
# This uses Porting/checkcfgvar.pl to check that none of the config.sh-like
# files are missing any entries.
#
# Why do we test this?
# We need them to be complete when we ship a release, and this way we catch
# problems as early as possible. (Instead of creating the potential for yet
# another last-minute job for the release manager). If a config file for a
# platform is incomplete, it can't be used to correctly regenerate config.h,
# because missing values result in invalid C code. We keep the files sorted
# as it makes it easy to automate adding defaults.
#
# It's broken - how do I fix it?
# The most likely reason that the test failed is because you've just added
# a new entry to Configure, config.sh and config_h.SH but nowhere else.
# Run something like:
#   perl Porting/checkcfgvar.pl --regen --default=undef
# (the correct default might not always be undef) to do most of the work, and
# then hand-edit configure.com (as that's not automated).
# If this changes uconfig.sh, you'll also need to run perl regen/uconfig_h.pl

use Config;
BEGIN {
    require "./test.pl";
    skip_all("Won't ship a release from EBCDIC") if $::IS_EBCDIC;
    @INC = '..' if -f '../TestInit.pm';
}
use TestInit qw(T A); # T is chdir to the top level, A makes paths absolute

if ( $Config{usecrosscompile} ) {
  skip_all( "Not all files are available during cross-compilation" );
}

system "$^X -Ilib Porting/checkcfgvar.pl --tap";
