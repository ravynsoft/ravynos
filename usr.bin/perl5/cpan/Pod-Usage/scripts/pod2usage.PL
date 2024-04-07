#!/usr/local/bin/perl

use strict;
use warnings;
use Config;
use File::Basename qw(&basename &dirname);
use Cwd;

# List explicitly here the variables you want Configure to
# generate.  Metaconfig only looks for shell variables, so you
# have to mention them as if they were shell variables, not
# %Config entries.  Thus you write
#  $startperl
# to ensure Configure will look for $Config{startperl}.

# This forces PL files to create target in same directory as PL file.
# This is so that make depend always knows where to find PL derivatives.
my $origdir = cwd;
chdir( dirname($0) );
my $file = basename( $0, '.PL' );
$file .= '.com' if $^O eq 'VMS';

open my $OUT, '>', $file or die "Can't create $file: $!";

print "Extracting $file (with variable substitutions)\n";

# In this section, perl variables will be expanded during extraction.
# You can use $Config{...} to use Configure variables.

print {$OUT} <<"!GROK!THIS!";
$Config{'startperl'}
    eval 'exec perl -S \$0 "\$@"'
        if 0;
!GROK!THIS!

# In the following, perl variables are not expanded during extraction.

print {$OUT} <<'!NO!SUBS!';

#############################################################################
# pod2usage -- command to print usage messages from embedded pod docs
#
# Copyright (c) 1996-2000 by Bradford Appleton. All rights reserved.
# Copyright (c) 2001-2016 by Marek Rouchal.
# This file is part of "Pod-Usage". Pod-Usage is free software;
# you can redistribute it and/or modify it under the same terms
# as Perl itself.
#############################################################################

use strict;
#use diagnostics;

=head1 NAME

pod2usage - print usage messages from embedded pod docs in files

=head1 SYNOPSIS

=over 12

=item B<pod2usage>

[B<-help>]
[B<-man>]
[B<-exit>S< >I<exitval>]
[B<-output>S< >I<outfile>]
[B<-verbose> I<level>]
[B<-pathlist> I<dirlist>]
[B<-formatter> I<module>]
[B<-utf8>]
I<file>

=back

=head1 OPTIONS AND ARGUMENTS

=over 8

=item B<-help>

Print a brief help message and exit.

=item B<-man>

Print this command's manual page and exit.

=item B<-exit> I<exitval>

The exit status value to return.

=item B<-output> I<outfile>

The output file to print to. If the special names "-" or ">&1" or ">&STDOUT"
are used then standard output is used. If ">&2" or ">&STDERR" is used then
standard error is used.

=item B<-verbose> I<level>

The desired level of verbosity to use:

    1 : print SYNOPSIS only
    2 : print SYNOPSIS sections and any OPTIONS/ARGUMENTS sections
    3 : print the entire manpage (similar to running pod2text)

=item B<-pathlist> I<dirlist>

Specifies one or more directories to search for the input file if it
was not supplied with an absolute path. Each directory path in the given
list should be separated by a ':' on Unix (';' on MSWin32 and DOS).

=item B<-formatter> I<module>

Which text formatter to use. Default is L<Pod::Text>, or for very old
Perl versions L<Pod::PlainText>. An alternative would be e.g. 
L<Pod::Text::Termcap>.

=item B<-utf8>

This option assumes that the formatter (see above) understands the option
"utf8". It turns on generation of utf8 output.

=item I<file>

The pathname of a file containing pod documentation to be output in
usage message format. If omitted, standard input is read - but the
output is then formatted with L<Pod::Text> only - unless a specific
formatter has been specified with B<-formatter>.

=back

=head1 DESCRIPTION

B<pod2usage> will read the given input file looking for pod
documentation and will print the corresponding usage message.
If no input file is specified then standard input is read.

B<pod2usage> invokes the B<pod2usage()> function in the B<Pod::Usage>
module. Please see L<Pod::Usage/pod2usage()>.

=head1 SEE ALSO

L<Pod::Usage>, L<pod2text>, L<Pod::Text>, L<Pod::Text::Termcap>,
L<perldoc>

=head1 AUTHOR

Please report bugs using L<http://rt.cpan.org>.

Brad Appleton E<lt>bradapp@enteract.comE<gt>

Based on code for B<pod2text(1)> written by
Tom Christiansen E<lt>tchrist@mox.perl.comE<gt>

=cut

use Getopt::Long;

## Define options
my %options = ();
my @opt_specs = (
    'help',
    'man',
    'exit=i',
    'output=s',
    'pathlist=s',
    'formatter=s',
    'verbose=i',
    'utf8!'
);

## Parse options
GetOptions(\%options, @opt_specs)  ||  pod2usage(2);
$Pod::Usage::Formatter = $options{formatter} if $options{formatter};
require Pod::Usage;
Pod::Usage->import();
pod2usage(1)  if ($options{help});
pod2usage(VERBOSE => 2)  if ($options{man});

## Dont default to STDIN if connected to a terminal
pod2usage(2) if ((@ARGV == 0) && (-t STDIN));

if (@ARGV > 1) {
    print STDERR "pod2usage: Too many filenames given\n\n";
    pod2usage(2);
}

my %usage = ();
$usage{-input}    = shift(@ARGV) || \*STDIN;
$usage{-exitval}  = $options{'exit'}      if (defined $options{'exit'});
$usage{-output}   = $options{'output'}    if (defined $options{'output'});
$usage{-verbose}  = $options{'verbose'}   if (defined $options{'verbose'});
$usage{-pathlist} = $options{'pathlist'}  if (defined $options{'pathlist'});
$usage{-utf8}     = $options{'utf8'}      if (defined $options{'utf8'});

pod2usage(\%usage);


!NO!SUBS!

close($OUT) or die "Can't close $file: $!";
chmod( 0755, $file ) or die "Can't reset permissions for $file: $!\n";
exec("$Config{'eunicefix'} $file") if $Config{'eunicefix'} ne ':';
chdir($origdir);
