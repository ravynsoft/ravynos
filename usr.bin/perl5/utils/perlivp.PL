#!/usr/local/bin/perl

use Config;
use File::Basename;
use Cwd;

# List explicitly here the variables you want Configure to
# generate.  Metaconfig only looks for shell variables, so you
# have to mention them as if they were shell variables, not
# %Config entries:
#  $startperl
#  $perlpath
#  $eunicefix

# This forces PL files to create target in same directory as PL file.
# This is so that make depend always knows where to find PL derivatives.
my $origdir = cwd;
chdir dirname($0);
my $file = basename($0, '.PL');
$file .= '.com' if $^O eq 'VMS';

# Create output file.
open OUT, ">", $file or die "Can't create $file: $!";

print "Extracting $file (with variable substitutions)\n";

# In this section, perl variables will be expanded during extraction.
# You can use $Config{...} to use Configure variables.

print OUT <<"!GROK!THIS!";
$Config{'startperl'}
    eval 'exec $Config{'perlpath'} -S \$0 \${1+"\$@"}'
        if 0; # ^ Run only under a shell
!GROK!THIS!

print OUT "\n# perlivp $^V\n";

# In the following, perl variables are not expanded during extraction.

print OUT <<'!NO!SUBS!';

BEGIN { pop @INC if $INC[-1] eq '.' }

sub usage {
    warn "@_\n" if @_;
    print << "    EOUSAGE";
Usage:

    $0 [-p] [-v] | [-h]

    -p Print a preface before each test telling what it will test.
    -v Verbose mode in which extra information about test results
       is printed.  Test failures always print out some extra information
       regardless of whether or not this switch is set.
    -h Prints this help message.
    EOUSAGE
    exit;
}

use vars qw(%opt); # allow testing with older versions (do not use our)

@opt{ qw/? H h P p V v/ } = qw(0 0 0 0 0 0 0);

while ($ARGV[0] =~ /^-/) {
    $ARGV[0] =~ s/^-//; 
    for my $flag (split(//,$ARGV[0])) {
        usage() if '?' =~ /\Q$flag/;
        usage() if 'h' =~ /\Q$flag/;
        usage() if 'H' =~ /\Q$flag/;
        usage("unknown flag: '$flag'") unless 'HhPpVv' =~ /\Q$flag/;
        warn "$0: '$flag' flag already set\n" if $opt{$flag}++;
    } 
    shift;
}

$opt{p}++ if $opt{P};
$opt{v}++ if $opt{V};

my $pass__total = 0;
my $error_total = 0;
my $tests_total = 0;

!NO!SUBS!

# We cannot merely check the variable '$^X' in general since on many 
# Unixes it is the basename rather than the full path to the perl binary.
my $perlpath = '';
if (defined($Config{'perlpath'})) { $perlpath = $Config{'perlpath'}; }

# The useithreads Config variable plays a role in whether or not
# threads and threads/shared work when C<use>d.  They apparently always
# get installed on systems that can run Configure.
my $useithreads = '';
if (defined($Config{'useithreads'})) { $useithreads = $Config{'useithreads'}; }

print OUT <<"!GROK!THIS!";
my \$perlpath = '$perlpath';
my \$useithreads = '$useithreads';
!GROK!THIS!

print OUT <<'!NO!SUBS!';

print "## Checking Perl binary via variable '\$perlpath' = $perlpath.\n" if $opt{'p'};

my $label = 'Executable perl binary';

if (-x $perlpath) {
    print "## Perl binary '$perlpath' appears executable.\n" if $opt{'v'};
    print "ok 1 $label\n";
    $pass__total++;
}
else {
    print "# Perl binary '$perlpath' does not appear executable.\n";
    print "not ok 1 $label\n";
    $error_total++;
}
$tests_total++;


print "## Checking Perl version via variable '\$]'.\n" if $opt{'p'};

!NO!SUBS!

print OUT <<"!GROK!THIS!";
my \$ivp_VERSION = "$]";

!GROK!THIS!
print OUT <<'!NO!SUBS!';

$label = 'Perl version correct';
if ($ivp_VERSION eq $]) {
    print "## Perl version '$]' appears installed as expected.\n" if $opt{'v'};
    print "ok 2 $label\n";
    $pass__total++;
}
else {
    print "# Perl version '$]' installed, expected $ivp_VERSION.\n";
    print "not ok 2 $label\n";
    $error_total++;
}
$tests_total++;

# We have the right perl and version, so now reset @INC so we ignore
# PERL5LIB and '.'
{
    local $ENV{PERL5LIB};
    my $perl_V = qx($perlpath -V);
    $perl_V =~ s{.*\@INC:\n}{}ms;
    @INC = grep { length && $_ ne '.' } split ' ', $perl_V;
}

print "## Checking roots of the Perl library directory tree via variable '\@INC'.\n" if $opt{'p'};

my $INC_total = 0;
my $INC_there = 0;
foreach (@INC) {
    next if $_ eq '.'; # skip -d test here
    if (-d $_) {
        print "## Perl \@INC directory '$_' exists.\n" if $opt{'v'};
        $INC_there++;
    }
    else {
        print "# Perl \@INC directory '$_' does not appear to exist.\n";
    }
    $INC_total++;
}

$label = '@INC directories exist';
if ($INC_total == $INC_there) {
    print "ok 3 $label\n";
    $pass__total++;
}
else {
    print "not ok 3 $label\n";
    $error_total++;
}
$tests_total++;


print "## Checking installations of modules necessary for ivp.\n" if $opt{'p'};

my $needed_total = 0;
my $needed_there = 0;
foreach (qw(Config.pm ExtUtils/Installed.pm)) {
    $@ = undef;
    $needed_total++;
    eval "require \"$_\";";
    if (!$@) {
        print "## Module '$_' appears to be installed.\n" if $opt{'v'};
        $needed_there++;
    }
    else {
        print "# Needed module '$_' does not appear to be properly installed.\n";
    }
    $@ = undef;
}
$label = 'Modules needed for rest of perlivp exist';
if ($needed_total == $needed_there) {
    print "ok 4 $label\n";
    $pass__total++;
}
else {
    print "not ok 4 $label\n";
    $error_total++;
}
$tests_total++;


print "## Checking installations of extensions built with perl.\n" if $opt{'p'};

use Config;

my $extensions_total = 0;
my $extensions_there = 0;
if (defined($Config{'extensions'})) {
    my @extensions = split(/\s+/,$Config{'extensions'});
    foreach (@extensions) {
        next if ($_ eq '');
        if ( $useithreads !~ /define/i ) {
            next if ($_ eq 'threads');
            next if ($_ eq 'threads/shared');
        }
        # that's a distribution name, not a module name
        next if $_ eq 'IO/Compress';
        next if $_ eq 'Devel/DProf';
        next if $_ eq 'libnet';
        next if $_ eq 'Locale/Codes';
        next if $_ eq 'podlators';
        next if $_ eq 'perlfaq';
        # test modules
        next if $_ eq 'XS/APItest';
        next if $_ eq 'XS/Typemap';
           # VMS$ perl  -e "eval ""require \""Devel/DProf.pm\"";"" print $@"
           # \NT> perl  -e "eval \"require './Devel/DProf.pm'\"; print $@"
           # DProf: run perl with -d to use DProf.
           # Compilation failed in require at (eval 1) line 1.
        eval " require \"$_.pm\"; ";
        if (!$@) {
            print "## Module '$_' appears to be installed.\n" if $opt{'v'};
            $extensions_there++;
        }
        else {
            print "# Required module '$_' does not appear to be properly installed.\n";
            $@ = undef;
        }
        $extensions_total++;
    }

    # A silly name for a module (that hopefully won't ever exist).
    # Note that this test serves more as a check of the validity of the
    # actual required module tests above.
    my $unnecessary = 'bLuRfle';

    if (!grep(/$unnecessary/, @extensions)) {
        $@ = undef;
        eval " require \"$unnecessary.pm\"; ";
        if ($@) {
            print "## Unnecessary module '$unnecessary' does not appear to be installed.\n" if $opt{'v'};
        }
        else {
            print "# Unnecessary module '$unnecessary' appears to be installed.\n";
            $extensions_there++;
        }
    }
    $@ = undef;
}
$label = 'All (and only) expected extensions installed';
if ($extensions_total == $extensions_there) {
    print "ok 5 $label\n";
    $pass__total++;
}
else {
    print "not ok 5 $label\n";
    $error_total++;
}
$tests_total++;


print "## Checking installations of later additional extensions.\n" if $opt{'p'};

use ExtUtils::Installed;

my $installed_total = 0;
my $installed_there = 0;
my $version_check = 0;
my $installed = ExtUtils::Installed -> new();
my @modules = $installed -> modules();
my @missing = ();
my $version = undef;
for (@modules) {
    $installed_total++;
    # Consider it there if it contains one or more files,
    # and has zero missing files,
    # and has a defined version
    $version = undef;
    $version = $installed -> version($_);
    if ($version) {
        print "## $_; $version\n" if $opt{'v'};
        $version_check++;
    }
    else {
        print "# $_; NO VERSION\n" if $opt{'v'};
    }
    $version = undef;
    @missing = ();
    @missing = $installed -> validate($_);

    # .bs files are optional
    @missing = grep { ! /\.bs$/ } @missing;
    # man files are often compressed
    @missing = grep { ! ( -s "$_.gz" || -s "$_.bz2" ) } @missing;

    if ($#missing >= 0) {
        print "# file",+($#missing == 0) ? '' : 's'," missing from installation:\n";
        print '# ',join(' ',@missing),"\n";
    }
    elsif ($#missing == -1) {
        $installed_there++;
    }
    @missing = ();
}
$label = 'Module files correctly installed';
if (($installed_total == $installed_there) && 
    ($installed_total == $version_check)) {
    print "ok 6 $label\n";
    $pass__total++;
}
else {
    print "not ok 6 $label\n";
    $error_total++;
}
$tests_total++;

# Final report (rather than feed ousrselves to Test::Harness::runtests()
# we simply format some output on our own to keep things simple and
# easier to "fix" - at least for now.

if ($error_total == 0 && $tests_total) {
    print "All tests successful.\n";
} elsif ($tests_total==0){
        die "FAILED--no tests were run for some reason.\n";
} else {
    my $rate = 0.0;
    if ($tests_total > 0) { $rate = sprintf "%.2f", 100.0 * ($pass__total / $tests_total); }
    printf " %d/%d subtests failed, %.2f%% okay.\n",
                              $error_total, $tests_total, $rate;
}

=head1 NAME

perlivp - Perl Installation Verification Procedure

=head1 SYNOPSIS

B<perlivp> [B<-p>] [B<-v>] [B<-h>]

=head1 DESCRIPTION

The B<perlivp> program is set up at Perl source code build time to test the
Perl version it was built under.  It can be used after running:

    make install

(or your platform's equivalent procedure) to verify that B<perl> and its
libraries have been installed correctly.  A correct installation is verified
by output that looks like:

    ok 1
    ok 2

etc.

=head1 OPTIONS

=over 5

=item B<-h> help

Prints out a brief help message.

=item B<-p> print preface

Gives a description of each test prior to performing it.

=item B<-v> verbose

Gives more detailed information about each test, after it has been performed.
Note that any failed tests ought to print out some extra information whether
or not -v is thrown.

=back

=head1 DIAGNOSTICS

=over 4

=item * print "# Perl binary '$perlpath' does not appear executable.\n";

Likely to occur for a perl binary that was not properly installed.
Correct by conducting a proper installation.

=item * print "# Perl version '$]' installed, expected $ivp_VERSION.\n";

Likely to occur for a perl that was not properly installed.
Correct by conducting a proper installation.

=item * print "# Perl \@INC directory '$_' does not appear to exist.\n";

Likely to occur for a perl library tree that was not properly installed.
Correct by conducting a proper installation.

=item * print "# Needed module '$_' does not appear to be properly installed.\n";

One of the two modules that is used by perlivp was not present in the 
installation.  This is a serious error since it adversely affects perlivp's
ability to function.  You may be able to correct this by performing a
proper perl installation.

=item * print "# Required module '$_' does not appear to be properly installed.\n";

An attempt to C<eval "require $module"> failed, even though the list of 
extensions indicated that it should succeed.  Correct by conducting a proper 
installation.

=item * print "# Unnecessary module 'bLuRfle' appears to be installed.\n";

This test not coming out ok could indicate that you have in fact installed 
a bLuRfle.pm module or that the C<eval " require \"$module_name.pm\"; ">
test may give misleading results with your installation of perl.  If yours
is the latter case then please let the author know.

=item * print "# file",+($#missing == 0) ? '' : 's'," missing from installation:\n";

One or more files turned up missing according to a run of 
C<ExtUtils::Installed -E<gt> validate()> over your installation.
Correct by conducting a proper installation.

=back

For further information on how to conduct a proper installation consult the 
INSTALL file that comes with the perl source and the README file for your 
platform.

=head1 AUTHOR

Peter Prymmer

=cut

!NO!SUBS!

close OUT or die "Can't close $file: $!";
chmod 0755, $file or die "Can't reset permissions for $file: $!\n";
exec("$Config{'eunicefix'} $file") if $Config{'eunicefix'} ne ':';
chdir $origdir;

