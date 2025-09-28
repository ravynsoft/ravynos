#!perl

=head1 NAME

copyright.t

=head1 DESCRIPTION

Tests that the latest copyright years in the top-level README file and the
C<perl -v> output match each other.

If the test fails, update at least one of README and perl.c so that they match
reality.

Optionally you can pass the C<--now> option to check they are at the current
year. This isn't checked by default, so that it doesn't fail for people
working on older releases. It should be run before making a new release.

=cut

use strict;
use Config;
BEGIN { require './test.pl' }

if ( $Config{usecrosscompile} ) {
  skip_all( "Not all files are available during cross-compilation" );
}

my ($opt) = @ARGV;

my $readme_year = readme_year();
my $v_year = v_year();
my $gh_readme_year;
# git on windows renders symbolic links as a file containing
# the file linked to
if (-e "../.github/README.md" && -s "../.github/README.md" > 80)
{
  $gh_readme_year = readme_year(".github/README.md");
}


# Check that both copyright dates are up-to-date, but only if requested, so
# that tests still pass for people intentionally working on older versions:
if ($opt eq '--now')
{
  my $current_year = (gmtime)[5] + 1900;
  is $v_year, $current_year, 'perl -v copyright includes current year';
  is $readme_year, $current_year, 'README copyright includes current year';
  if ($gh_readme_year)
  {
    is ($gh_readme_year, $current_year,
        '.github/README.md copyright includes current year');
  }
}

# Otherwise simply check that the two copyright dates match each other:
else
{
  is $readme_year, $v_year, 'README and perl -v copyright dates match';
  if ($gh_readme_year)
  {
    is ($gh_readme_year, $v_year,
        '.github/README.md and perl -v copyright dates match');
  }
}

done_testing;


sub readme_year
# returns the latest copyright year from the top-level README file
{
  my $file = shift || "README";

  open my $readme, '<', "../$file" or die "Opening $file failed: $!";

  # The copyright message is the first paragraph:
  local $/ = '';
  my $copyright_msg = <$readme>;

  my ($year) = $copyright_msg =~ /.*\b(\d{4,})/s
      or die "Year not found in $file copyright message '$copyright_msg'";

  $year;
}


sub v_year
# returns the latest copyright year shown in perl -v
{

  my $output = runperl switches => ['-v'];
  my ($year) = $output =~ /copyright 1987.*\b(\d{4,})/i
      or die "Copyright statement not found in perl -v output '$output'";

  $year;
}
