#!perl -w
use 5.010;
use strict;
use Getopt::Long;
use Pod::Usage;
use Module::CoreList;
GetOptions(
    'spec|s:s' => \my $manifest,
) or pod2usage();

=head1 SYNOPSIS

  newtests-perldelta.pl [FROM TO]

Output the added tests between the two last released versions of Perl

  newtests-perldelta.pl

Output the added tests between the version tagged v5.11.1
and this version

  newtests-perldelta.pl v5.11.1 HEAD

=cut

my $corelist = \%Module::CoreList::version;
my @versions = sort keys %$corelist;

# by default, compare latest two version in CoreList;
my ($from, $to) = @ARGV;

# Convert the numbers to git version tags
sub num2git {
    my ($num) = @_;
    $num =~ /^(5)\.(\d\d\d)(\d\d\d)/
        or die "Couldn't make sense of version number '$num'";
    sprintf 'v%d.%d.%d', $1,$2,$3;
};

$from //= num2git($versions[-2]); # /
$to   //= num2git($versions[-1]); # /
$manifest //= 'MANIFEST'; # /

warn "Finding newly added tests between $from..$to\n";

my @new_tests =
    grep { m!^[-+](?:t|ext)/.*\.t\s+! } # only added/removed tests
    `git diff $from..$to -- $manifest`;
chomp @new_tests;
if (! @new_tests) {
    die "No new tests found between $from and $to.";
};

# Now remove those files whose lines were just shuffled around
# within MANIFEST
my %desc;
my %removed;
for (@new_tests) {
    die "Weird diff line '$_' " unless /^([+-])(\S+\.t)(?:\s+(.*))?$/;
    my ($mod,$file,$desc) = ($1,$2,$3);
    $desc //= '<no description>'; # / to placate Padre highlighting
    if ($mod eq '-') {
        $removed{ $file } = $file;
    };
    $desc{ $file } = $desc;
};

print <<HEAD;

=head1 New Tests

Many modules updated from CPAN incorporate new tests.

=over 4

HEAD

for my $file (sort keys %desc) {
    next if $removed{ $file };
    print <<ITEM;
=item $file

$desc{ $file }

ITEM
};

print <<TAIL

=back

TAIL
