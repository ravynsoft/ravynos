#!/usr/bin/perl -w

# Here we make sure File::Spec can properly deal with executables.
# VMS has some trouble with these.

use File::Spec;
use lib File::Spec->catdir('t', 'lib');

use Test::More (-x $^X
		? (tests => 5)
		: (skip_all => "Can't find an executable file")
	       );

BEGIN {                                # Set up a tiny script file
    local *F;
    open(F, ">rel2abs2rel$$.pl")
      or die "Can't open rel2abs2rel$$.pl file for script -- $!\n";
    print F qq(print "ok\\n"\n);
    close(F);
}
END {
    1 while unlink("rel2abs2rel$$.pl");
    1 while unlink("rel2abs2rel$$.tmp");
}

use Config;


# Change 'perl' to './perl' so the shell doesn't go looking through PATH.
sub safe_rel {
    my($perl) = shift;
    $perl = File::Spec->catfile(File::Spec->curdir, $perl) unless
      File::Spec->file_name_is_absolute($perl);

    return $perl;
}
# Make a putative perl binary say "ok\n". We have to do it this way
# because the filespec of the binary may contain characters that a
# command interpreter considers special, so we can't use the obvious
# `$perl -le "print 'ok'"`. And, for portability, we can't use fork().
sub sayok{
    my $perl = shift;
    open(STDOUTDUP, '>&STDOUT');
    open(STDOUT, ">rel2abs2rel$$.tmp")
        or die "Can't open scratch file rel2abs2rel$$.tmp -- $!\n";
    system($perl, "rel2abs2rel$$.pl");
    open(STDOUT, '>&STDOUTDUP');
    close(STDOUTDUP);

    local *F;
    open(F, "rel2abs2rel$$.tmp");
    local $/ = undef;
    my $output = <F>;
    close(F);
    return $output;
}

print "# Checking manipulations of \$^X=$^X\n";

my $perl = safe_rel($^X);
is( sayok($perl), "ok\n",   "'$perl rel2abs2rel$$.pl' works" );

$perl = File::Spec->rel2abs($^X);
is( sayok($perl), "ok\n",   "'$perl rel2abs2rel$$.pl' works" );

$perl = File::Spec->canonpath($perl);
is( sayok($perl), "ok\n",   "canonpath(rel2abs($^X)) = $perl" );

$perl = safe_rel(File::Spec->abs2rel($perl));
is( sayok($perl), "ok\n",   "safe_rel(abs2rel(canonpath(rel2abs($^X)))) = $perl" );

$perl = safe_rel(File::Spec->canonpath($^X));
is( sayok($perl), "ok\n",   "safe_rel(canonpath($^X)) = $perl" );
