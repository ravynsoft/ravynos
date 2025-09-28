#!/usr/bin/perl
#
# Check or update the version of Perl modules.
#
# Examines all module files (*.pm) under the lib directory and verifies that
# the package is set to the same value as the current version number as
# determined by the MYMETA.json file at the top of the source distribution.
#
# When given the --update option, instead fixes all of the Perl modules found
# to have the correct version.
#
# SPDX-License-Identifier: MIT

use 5.010;
use strict;
use warnings;

use lib 't/lib';

use Test::RRA qw(skip_unless_automated use_prereq);
use Test::RRA::ModuleVersion qw(test_module_versions update_module_versions);

use Getopt::Long qw(GetOptions);

# If we have options, we're being run from the command line and always load
# our prerequisite modules.  Otherwise, check if we have necessary
# prerequisites and should run as a test suite.
if (@ARGV) {
    require JSON::PP;
    require Perl6::Slurp;
    Perl6::Slurp->import;
} else {
    skip_unless_automated('Module version tests');
    use_prereq('JSON::PP');
    use_prereq('Perl6::Slurp');
}

# Return the current version of the distribution from MYMETA.json in the
# current directory.
#
# Returns: The version number of the distribution
# Throws: Text exception if MYMETA.json is not found or doesn't contain a
#         version
sub dist_version {
    my $json = JSON::PP->new->utf8(1);
    my $metadata = $json->decode(scalar(slurp('MYMETA.json')));
    my $version = $metadata->{version};
    if (!defined($version)) {
        die "$0: cannot find version number in MYMETA.json\n";
    }
    return $version;
}

# Get the version of the overall distribution.
my $version = dist_version();

# Main routine.  We run as either a test suite or as a script to update all of
# the module versions, selecting based on whether we got the -u / --update
# command-line option.
my $update;
Getopt::Long::config('bundling', 'no_ignore_case');
GetOptions('update|u' => \$update) or exit 1;
if ($update) {
    update_module_versions('lib', $version);
} else {
    test_module_versions('lib', $version);
}
exit 0;
__END__

=for stopwords
Allbery sublicense MERCHANTABILITY NONINFRINGEMENT CPAN rra-c-util

=head1 NAME

module-version.t - Check or update versions of Perl modules

=head1 SYNOPSIS

B<module-version.t> [B<--update>]

=head1 REQUIREMENTS

Perl 5.8 or later, the Perl6::Slurp module, and the JSON::PP Perl module, both
of which are available from CPAN.  JSON::PP is also included in Perl core in
Perl 5.14 and later.

=head1 DESCRIPTION

This script has a dual purpose as either a test script or a utility script.
The intent is to assist with maintaining consistent versions in a Perl
distribution, supporting both the package keyword syntax introduced in Perl
5.12 or the older explicit setting of a $VERSION variable.

As a test, it reads the current version of a package from the F<MYMETA.json>
file in the current directory (which should be the root of the distribution)
and then looks for any Perl modules in F<lib>.  If it finds any, it checks
that the version number of the Perl module matches the version number of the
package from the F<MYMETA.json> file.  These test results are reported with
Test::More, suitable for any TAP harness.

As a utility script, when run with the B<--update> option, it similarly finds
all Perl modules in F<lib> and then rewrites their version setting to match
the version of the package as determined from the F<MYMETA.json> file.

=head1 OPTIONS

=over 4

=item B<-u>, B<--update>

Rather than test the Perl modules for the correct version, update all Perl
modules found in the tree under F<lib> to the current version from the
C<MYMETA.json> file.

=back

=head1 AUTHOR

Russ Allbery <eagle@eyrie.org>

=head1 COPYRIGHT AND LICENSE

Copyright 2014-2016, 2019-2021 Russ Allbery <eagle@eyrie.org>

Copyright 2013-2014 The Board of Trustees of the Leland Stanford Junior
University

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

=head1 SEE ALSO

This module is maintained in the rra-c-util package.  The current version
is available from L<https://www.eyrie.org/~eagle/software/rra-c-util/>.

=cut

# Local Variables:
# copyright-at-end-flag: t
# End:
