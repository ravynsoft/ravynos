#!./perl -w
# Test that there are no missing authors in AUTHORS

BEGIN {
    chdir 't' if -d 't';
    require "./test.pl";
    set_up_inc('../lib', '..');
}

use TestInit qw(T);    # T is chdir to the top level
use strict;

my $source_dir = find_git_or_skip('all');
skip_all(
    "This distro may have modified some files in cpan/. Skipping validation.")
  if $ENV{'PERL_BUILD_PACKAGING'};

skip_all(
    "This is a shallow clone, this test requires history.")
  if (-e "$source_dir/.git/shallow");

my $revision_range = ''; # could use 'v5.22.0..' as default, no reason to recheck all previous commits...
if ( $ENV{TRAVIS} && defined $ENV{TRAVIS_COMMIT_RANGE} ) {
	# travisci is adding a merge commit when smoking a pull request
	#	unfortunately it's going to use the default GitHub email from the author
	#	which can differ from the one the author wants to use as part of the pull request
	#	let's simply use the TRAVIS_COMMIT_RANGE which list the commits we want to check
	#	all the more a pull request should not be impacted by blead being incorrect
	$revision_range = $ENV{TRAVIS_COMMIT_RANGE};
}
elsif( $ENV{GITHUB_ACTIONS} && length $ENV{GITHUB_BASE_REF} ) {
    # Same as above, except for GitHub Actions
    # https://help.github.com/en/actions/automating-your-workflow-with-github-actions/using-environment-variables

    # we should be on a merge commit, but double check
    my $branch_head = `git rev-parse -q --verify "HEAD^2"`;
    chomp $branch_head;

    # gives the history of the branch being merged, excluding what it is
    # merged into
    $revision_range = '"HEAD^1..HEAD^2"'
        if $branch_head;
}

exec("$^X Porting/updateAUTHORS.pl --validate $revision_range");
# EOF
