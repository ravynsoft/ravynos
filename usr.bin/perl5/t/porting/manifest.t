#!./perl -w

# What does this test?
# This tests the well-formed-ness of the MANIFEST file.
#
# Why do we test this?
# TK
#
# It's broken - how do I fix it?
# If MANIFEST is not sorted properly, you will get this error output:
#      got ''MANIFEST' is NOT sorted properly
#      # '
#      # expected /(?^:is sorted properly)/
#
# To correct this, run either:
#
#   ./perl -Ilib Porting/manisort -o MANIFEST MANIFEST
#
# which will output "'MANIFEST' is NOT sorted properly" but which will
# correct the problem; or:
#
#   make manisort
#
# which will output "WARNING: re-sorting MANIFEST" but which will also
# correct the problem.

use Config;
BEGIN {
    @INC = '..' if -f '../TestInit.pm';
}
use TestInit qw(T); # T is chdir to the top level

require './t/test.pl';

skip_all("Cross-compiling, the entire source might not be available")
    if $Config{usecrosscompile};


plan('no_plan');

my $manifest = 'MANIFEST';

open my $m, '<', $manifest or die "Can't open '$manifest': $!";
my @files;
# Test that MANIFEST uses tabs - not spaces - after the name of the file.
while (<$m>) {
    chomp;
    unless( /\s/ ) {
        push @files, $_;
        # no need for further tests on lines without whitespace (i.e., filename only)
        next;
    }
    my ($file, $separator) = /^(\S+)(\s+)/;
    push @files, $file;

    isnt($file, undef, "Line $. doesn't start with a blank") or next;
    ok(-f $file, "File $file exists");
    if ($separator !~ tr/\t//c) {
	# It's all tabs
	next;
    } elsif ($separator !~ tr/ //c) {
	# It's all spaces
	fail("Spaces in entry for $file in MANIFEST at line $. (run `make manisort` to fix)");
    } elsif ($separator =~ tr/\t//) {
	fail("Mixed tabs and spaces in entry for $file in MANIFEST at line $. (run `make manisort` to fix)");
    } else {
	fail("Odd whitespace in entry for $file in MANIFEST at line $. (run `make manisort` to fix)");
    }
}

close $m or die $!;

# Test that MANIFEST is properly sorted
SKIP: {
    skip("Sorting order is different under EBCDIC", 1) if $::IS_EBCDIC || $::IS_EBCDIC;
    skip("'Porting/manisort' not found", 1) if (! -f 'Porting/manisort');

    my $result = runperl('progfile' => 'Porting/manisort',
                         'args'     => [ '-c', $manifest ],
                         'stderr'   => 1,
                         'nolib'    => 1 );

    like($result, qr/is sorted properly/, 'MANIFEST sorted properly');
}

SKIP: {
    find_git_or_skip(6);
    my %seen; # De-dup ls-files output (can appear more than once)
    my @repo= grep {
        chomp();
        !m{\.git_patch$} &&
        !m{\.gitattributes$} &&
        !m{\.gitignore$} &&
        !m{\.mailmap$} &&
        !m{^\.github/} &&
        -e $_ &&
        !$seen{$_}++
    } `git ls-files`;
    skip("git ls-files didnt work",3)
        if !@repo;
    is( 0+@repo, 0+@files, "git ls-files gives the same number of files as MANIFEST lists");
    my %repo;
    ++$repo{$_} for @repo;
    my %mani;
    ++$mani{$_} for @files;
    is( 0+keys %mani, 0+@files, "no duplicate files in MANIFEST")
      or diag(join("\n  ", "Duplicates:",grep $mani{$_} > 1, keys %mani));
    delete $mani{$_} for @repo;
    delete $repo{$_} for @files;
    my @not_in_mani= keys %repo;
    my @still_in_mani= keys %mani;

    is( 0+@not_in_mani, 0, "Nothing added to the repo that isn't in MANIFEST");
    is( "not in MANIFEST: @not_in_mani", "not in MANIFEST: ",
        "Nothing added to the repo that isn't in MANIFEST");
    is( 0+@still_in_mani, 0, "Nothing in the MANIFEST that isn't tracked by git");
    is( "should not be in MANIFEST: @still_in_mani", "should not be in MANIFEST: ",
        "Nothing in the MANIFEST that isn't tracked by git");

}

# EOF
