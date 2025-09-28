#!perl

=head1 NAME

Porting/acknowledgements.pl - Generate perldelta acknowledgements text

=head1 SYNOPSIS

  perl Porting/acknowledgements.pl v5.15.0..HEAD

=head1 DESCRIPTION

This generates the text which goes in the Acknowledgements section in
a perldelta. You pass in the previous version and it guesses the next
version, fetches information from the repository and outputs the
text.

=cut

use strict;
use warnings;
use autodie;
use POSIX qw(ceil);
use Text::Wrap;
use Time::Piece;
use Time::Seconds;
use version;
$Text::Wrap::columns = 77;

my $since_until = shift;

my ( $since, $until ) = split '\.\.', $since_until;

die "Usage: perl Porting/acknowledgements.pl v5.15.0..HEAD"
    unless $since_until && $since && $until;

my $previous_version = previous_version();
my $next_version     = next_version();
my $development_time = development_time();

my ( $changes, $files, $code_changes, $code_files ) = changes_files();
my $formatted_changes = commify( round($changes) );
my $formatted_files   = commify( round($files) );
my $formatted_code_changes = commify( round($code_changes) );
my $formatted_code_files   = commify( round($code_files) );

my $authors = authors();
my $nauthors = $authors =~ tr/,/,/;
$nauthors++;

my $text
    = "Perl $next_version represents approximately $development_time of development
since Perl $previous_version and contains approximately $formatted_changes
lines of changes across $formatted_files files from $nauthors authors.

Excluding auto-generated files, documentation and release tools, there
were approximately $formatted_code_changes lines of changes to
$formatted_code_files .pm, .t, .c and .h files.

Perl continues to flourish into its fourth decade thanks to a vibrant
community of users and developers. The following people are known to
have contributed the improvements that became Perl $next_version:

$authors
The list above is almost certainly incomplete as it is automatically
generated from version control history. In particular, it does not
include the names of the (very much appreciated) contributors who
reported issues to the Perl bug tracker.

Many of the changes included in this version originated in the CPAN
modules included in Perl's core. We're grateful to the entire CPAN
community for helping Perl to flourish.

For a more complete list of all of Perl's historical contributors,
please see the F<AUTHORS> file in the Perl source distribution.";

my $wrapped = fill( '', '', $text );
print "$wrapped\n";

# return the previous Perl version, eg 5.15.0
sub previous_version {
    my $version = version->new($since);
    $version =~ s/^v//;
    return $version;
}

# returns the upcoming release Perl version, eg 5.15.1
sub next_version {
    my $version = version->new($since);
    ( $version->{version}->[-1] )++;
    return version->new( join( '.', @{ $version->{version} } ) );
}

# returns the development time since the previous version in weeks
# or months
sub development_time {
    my $first_timestamp = qx(git log -1 --pretty=format:%ct --summary $since);
    my $last_timestamp  = qx(git log -1 --pretty=format:%ct --summary $until);

    die "Missing first timestamp" unless $first_timestamp;
    die "Missing last timestamp" unless $last_timestamp;

    my $seconds = localtime($last_timestamp) - localtime($first_timestamp);
    my $weeks   = _round( $seconds / ONE_WEEK );
    my $months  = _round( $seconds / ONE_MONTH );

    my $development_time;
    if ( $months < 2 ) {
        return "$weeks @{[$weeks == 1 ? q(week) : q(weeks)]}";
    } else {
        return "$months months";
    }
}

sub _round {
    my $val = shift;

    my $int = int $val;
    my $remainder = $val - $int;

    return $remainder >= 0.5 ? $int + 1 : $int;
}

# returns the number of changed lines and files since the previous
# version
sub changes_files {
    my $output = qx(git diff --shortstat $since_until);
    my $q = ($^O =~ /^(?:MSWin32|VMS)$/io) ? '"' : "'";
    my @filenames = qx(git diff --numstat $since_until | $^X -anle ${q}next if m{^dist/Module-CoreList} or not /\\.(?:pm|c|h|t)\\z/; print \$F[2]$q);
    chomp @filenames;
    my $output_code_changed = qx# git diff --shortstat $since_until -- @filenames #;

    return ( _changes_from_cmd ( $output ),
             _changes_from_cmd ( $output_code_changed ) );
}

sub _changes_from_cmd {
    my $output = shift || die "No git diff command output";

    # 585 files changed, 156329 insertions(+), 53586 deletions(-)
    my ( $files, $insertions, $deletions )
        = $output
        =~ /(\d+) files changed, (\d+) insertions\(\+\), (\d+) deletions\(-\)/;
    my $changes = $insertions + $deletions;
    return ( $changes, $files );
}

# rounds an integer to two significant figures
sub round {
    my $int     = shift;
    my $length  = length($int);
    my $divisor = 10**( $length - 2 );
    return ceil( $int / $divisor ) * $divisor;
}

# adds commas to a number at thousands, millions
sub commify {
    local $_ = shift;
    1 while s/^([-+]?\d+)(\d{3})/$1,$2/;
    return $_;
}

# returns a list of the authors
sub authors {
    return
        qx($^X Porting/updateAUTHORS.pl --who $since_until);
}
