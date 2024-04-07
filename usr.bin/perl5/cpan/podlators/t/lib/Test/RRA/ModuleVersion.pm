# Check Perl module versions for consistency.
#
# This module contains the common code for testing and updating Perl module
# versions for consistency within a Perl module distribution and within a
# larger package that contains both Perl modules and other code.
#
# SPDX-License-Identifier: MIT

package Test::RRA::ModuleVersion;

use 5.010;
use base qw(Exporter);
use strict;
use warnings;

use File::Find qw(find);
use Test::More;
use Test::RRA::Config qw(@MODULE_VERSION_IGNORE);

# Declare variables that should be set in BEGIN for robustness.
our (@EXPORT_OK, $VERSION);

# Set $VERSION and everything export-related in a BEGIN block for robustness
# against circular module loading (not that we load any modules, but
# consistency is good).
BEGIN {
    @EXPORT_OK = qw(test_module_versions update_module_versions);

    # This version should match the corresponding rra-c-util release, but with
    # two digits for the minor version, including a leading zero if necessary,
    # so that it will sort properly.
    $VERSION = '10.03';
}

# A regular expression matching the version string for a module using the
# package syntax from Perl 5.12 and later.  $1 will contain all of the line
# contents prior to the actual version string, $2 will contain the version
# itself, and $3 will contain the rest of the line.
our $REGEX_VERSION_PACKAGE = qr{
    (                           # prefix ($1)
        \A \s*                  # whitespace
        package \s+             # package keyword
        [\w\:\']+ \s+           # package name
    )
    ( v? [\d._]+ )              # the version number itself ($2)
    (                           # suffix ($3)
        \s* ;
    )
}xms;

# A regular expression matching a $VERSION string in a module.  $1 will
# contain all of the line contents prior to the actual version string, $2 will
# contain the version itself, and $3 will contain the rest of the line.
our $REGEX_VERSION_OLD = qr{
    (                           # prefix ($1)
        \A .*                   # any prefix, such as "our"
        [\$*]                   # scalar or typeglob
        [\w\:\']*\b             # optional package name
        VERSION\b               # version variable
        \s* = \s*               # assignment
    )
    [\"\']?                     # optional leading quote
    ( v? [\d._]+ )              # the version number itself ($2)
    [\"\']?                     # optional trailing quote
    (                           # suffix ($3)
        \s*
        ;
    )
}xms;

# Find all the Perl modules shipped in this package, if any, and returns the
# list of file names.
#
# $dir - The root directory to search
#
# Returns: List of file names
sub _module_files {
    my ($dir) = @_;
    return if !-d $dir;
    my @files;
    my %ignore = map { $_ => 1 } @MODULE_VERSION_IGNORE;
    my $wanted = sub {
        if ($_ eq 'blib') {
            $File::Find::prune = 1;
            return;
        }
        if (m{ [.] pm \z }xms && !$ignore{$File::Find::name}) {
            push(@files, $File::Find::name);
        }
        return;
    };
    find($wanted, $dir);
    return @files;
}

# Given a module file, read it for the version value and return the value.
#
# $file - File to check, which should be a Perl module
#
# Returns: The version of the module
#  Throws: Text exception on I/O failure or inability to find version
sub _module_version {
    my ($file) = @_;
    open(my $data, q{<}, $file) or die "$0: cannot open $file: $!\n";
    while (defined(my $line = <$data>)) {
        if ($line =~ $REGEX_VERSION_PACKAGE || $line =~ $REGEX_VERSION_OLD) {
            my ($prefix, $version, $suffix) = ($1, $2, $3);
            close($data) or die "$0: error reading from $file: $!\n";
            return $version;
        }
    }
    close($data) or die "$0: error reading from $file: $!\n";
    die "$0: cannot find version number in $file\n";
}

# Given a module file and the new version for that module, update the version
# in that module to the new one.
#
# $file    - Perl module file whose version should be updated
# $version - The new version number
#
# Returns: undef
#  Throws: Text exception on I/O failure or inability to find version
sub _update_module_version {
    my ($file, $version) = @_;

    # The old-style syntax may require different quoting.  If the version
    # starts with v, use it without quotes.  Otherwise, quote it to prevent
    # removal of trailing zeroes.
    my $old_version = $version;
    if ($old_version !~ m{ \A v }xms) {
        $old_version = "'$old_version'";
    }

    # Scan for the version and replace it.
    open(my $in, q{<}, $file) or die "$0: cannot open $file: $!\n";
    open(my $out, q{>}, "$file.new")
      or die "$0: cannot create $file.new: $!\n";
  SCAN:
    while (defined(my $line = <$in>)) {
        if ($line =~ s{ $REGEX_VERSION_PACKAGE }{$1$version$3}xms
            || $line =~ s{ $REGEX_VERSION_OLD }{$1$old_version$3}xms)
        {
            print {$out} $line or die "$0: cannot write to $file.new: $!\n";
            last SCAN;
        }
        print {$out} $line or die "$0: cannot write to $file.new: $!\n";
    }

    # Copy the rest of the input file to the output file.
    print {$out} <$in> or die "$0: cannot write to $file.new: $!\n";
    close($out) or die "$0: cannot flush $file.new: $!\n";
    close($in) or die "$0: error reading from $file: $!\n";

    # All done.  Rename the new file over top of the old file.
    rename("$file.new", $file)
      or die "$0: cannot rename $file.new to $file: $!\n";
    return;
}

# Act as a test suite.  Find all of the Perl modules under the provided root,
# if any, and check that the version for each module matches the version.
# Reports results with Test::More and sets up a plan based on the number of
# modules found.
#
# $root    - Directory under which to look for Perl modules
# $version - The version all those modules should have
#
# Returns: undef
#  Throws: Text exception on fatal errors
sub test_module_versions {
    my ($root, $version) = @_;
    my @modules = _module_files($root);

    # Output the plan.  Skip the test if there were no modules found.
    if (@modules) {
        plan tests => scalar(@modules);
    } else {
        plan skip_all => 'No Perl modules found';
        return;
    }

    # For each module, get the module version and compare.
    for my $module (@modules) {
        my $module_version = _module_version($module);
        is($module_version, $version, "Version for $module");
    }
    return;
}

# Update the versions of all modules to the current distribution version.
#
# $root    - Directory under which to look for Perl modules
# $version - The version all those modules should have
#
# Returns: undef
#  Throws: Text exception on fatal errors
sub update_module_versions {
    my ($root, $version) = @_;
    my @modules = _module_files($root);
    for my $module (@modules) {
        _update_module_version($module, $version);
    }
    return;
}

1;
__END__

=for stopwords
Allbery sublicense MERCHANTABILITY NONINFRINGEMENT rra-c-util versioning

=head1 NAME

Test::RRA::ModuleVersion - Check Perl module versions for consistency

=head1 SYNOPSIS

    use Test::RRA::ModuleVersion
      qw(test_module_versions update_module_versions);

    # Ensure all modules under perl/lib have a version of 3.12.
    test_module_versions('perl/lib', '3.12');

    # Update the version of those modules to 3.12.
    update_module_versions('perl/lib', 3.12');

=head1 DESCRIPTION

This module provides functions to test and update the versions of Perl
modules.  It helps with enforcing consistency of versioning across all modules
in a Perl distribution or embedded in a larger project containing non-Perl
code.  The calling script provides the version with which to be consistent
and the root directory under which modules are found.

=head1 FUNCTIONS

None of these functions are imported by default.  The ones used by a script
should be explicitly imported.

=over 4

=item test_module_versions(ROOT, VERSION)

Tests the version of all Perl modules under ROOT to ensure they match VERSION,
reporting the results with Test::More.  If the test configuration loaded by
Test::RRA::Config contains a @MODULE_VERSION_EXCLUDE variable, the module
files listed there will be ignored for this test.  This function also sets up
a plan based on the number of modules, so should be the only testing function
called in a test script.

=item update_module_versions(ROOT, VERSION)

Update the version of all Perl modules found under ROOT to VERSION, except for
any listed in a @MODULE_VERSION_EXCLUDE variable set in the test configuration
loaded by Test::RRA::Config.

=back

=head1 AUTHOR

Russ Allbery <eagle@eyrie.org>

=head1 COPYRIGHT AND LICENSE

Copyright 2016, 2018-2020, 2022 Russ Allbery <eagle@eyrie.org>

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

Test::More(3), Test::RRA::Config(3)

This module is maintained in the rra-c-util package.  The current version
is available from L<https://www.eyrie.org/~eagle/software/rra-c-util/>.

=cut

# Local Variables:
# copyright-at-end-flag: t
# End:
