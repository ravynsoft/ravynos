#!perl
# Reports, in a perl source tree, which dual-lived core modules have not the
# same version than the corresponding module on CPAN.
# with -t option, can compare multiple source trees in tabular form.

use 5.9.0;
use strict;
use Getopt::Std;
use ExtUtils::MM_Unix;
use lib 'Porting';
use Maintainers qw(get_module_files reload_manifest %Modules);
use Cwd;

use List::Util qw(max);

our $packagefile = '02packages.details.txt';

sub usage () {
    die <<USAGE;
$0
$0 -t home1[:label] home2[:label] ...

Report which core modules are outdated.
To be run at the root of a perl source tree.

Options :
-h : help
-v : verbose (print all versions of all files, not only those which differ)
-f : force download of $packagefile from CPAN
     (it's expected to be found in the current directory)
-t : display in tabular form CPAN vs one or more perl source trees
USAGE
}

sub get_package_details () {
    my $url = 'http://www.cpan.org/modules/02packages.details.txt.gz';
    unlink $packagefile;
    system("wget $url && gunzip $packagefile.gz") == 0
	or die "Failed to get package details\n";
}

getopts('fhvt');
our $opt_h and usage;
our $opt_t;

my @sources = @ARGV ? @ARGV : '.';
die "Too many directories specified without -t option\n"
    if @sources != 1 and ! $opt_t;

@sources = map {
		# handle /home/user/perl:bleed style labels
		my ($dir,$label) = split /:/;
		$label = $dir unless defined $label;
		[ $dir, $label ];
	    } @sources;

our $opt_f || !-f $packagefile and get_package_details;

# Load the package details. All of them.
my %cpanversions;
open my $fh, '<', $packagefile or die $!;
while (<$fh>) {
    my ($p, $v) = split ' ';
    next if 1../^\s*$/; # skip header
    $cpanversions{$p} = $v;
}
close $fh;

my %results;

# scan source tree(s) and CPAN module list, and put results in %results

foreach my $source (@sources) {
    my ($srcdir, $label) = @$source;
    my $olddir = getcwd();
    chdir $srcdir or die "chdir $srcdir: $!\n";

    # load the MANIFEST file in the new directory
    reload_manifest;

    for my $dist (sort keys %Modules) {
	next unless $Modules{$dist}{CPAN};
	for my $file (get_module_files($dist)) {
	    next if $file !~ /(\.pm|_pm.PL)\z/
			or $file =~ m{^t/} or $file =~ m{/t/};
	    my $vcore = '!EXIST';
	    $vcore = MM->parse_version($file) // 'undef' if -f $file;

	    # get module name from filename to lookup CPAN version
	    my $module = $file;
	    $module =~ s/\_pm.PL\z//;
	    $module =~ s/\.pm\z//;
	    # some heuristics to figure out the module name from the file name
	    $module =~ s{^(lib|ext|dist|cpan)/}{}
		and $1 =~ /(?:ext|dist|cpan)/
		and (
		      # ext/Foo-Bar/Bar.pm
		      $module =~ s{^(\w+)-(\w+)/\2$}{$1/lib/$1/$2},
		      # ext/Encode/Foo/Foo.pm
		      $module =~ s{^(Encode)/(\w+)/\2$}{$1/lib/$1/$2},
		      $module =~ s{^[^/]+/}{},
		      $module =~ s{^lib/}{},
		    );
	    $module =~ s{/}{::}g;
	    my $vcpan = $cpanversions{$module} // 'undef';
	    $results{$dist}{$file}{$label} = $vcore;
	    $results{$dist}{$file}{CPAN} = $vcpan;
	}
    }

    chdir $olddir or die "chdir $olddir: $!\n";
}

# output %results in the requested format

my @labels = ((map $_->[1], @sources), 'CPAN' );

if ($opt_t) {
    my %changed;
    my @fields;
    for my $dist (sort { lc $a cmp lc $b } keys %results) {
	for my $file (sort keys %{$results{$dist}}) {
	    my @versions = @{$results{$dist}{$file}}{@labels};
	    for (0..$#versions) {
		$fields[$_] = max($fields[$_],
				  length $versions[$_],
				  length $labels[$_],
				  length '!EXIST'
				);
	    }
	    if (our $opt_v or grep $_ ne $versions[0], @versions) {
		$changed{$dist} = 1;
	    }
	}
    }
    printf "%*s ", $fields[$_], $labels[$_] for 0..$#labels;
    print "\n";
    printf "%*s ", $fields[$_], '-' x length $labels[$_] for 0..$#labels;
    print "\n";

    my $field_total;
    $field_total += $_ + 1 for @fields;

    for my $dist (sort { lc $a cmp lc $b } keys %results) {
	next unless $changed{$dist};
	print " " x $field_total, " $dist\n";
	for my $file (sort keys %{$results{$dist}}) {
	    my @versions = @{$results{$dist}{$file}}{@labels};
	    for (0..$#versions) {
		printf "%*s ", $fields[$_], $versions[$_]//'!EXIST'
	    }
	    print "    $file\n";
	}
    }
}
else {
    for my $dist (sort { lc $a cmp lc $b } keys %results) {
	my $distname_printed = 0;
	for my $file (sort keys %{$results{$dist}}) {
	    my ($vcore, $vcpan) = @{$results{$dist}{$file}}{@labels};
	    if (our $opt_v or $vcore ne $vcpan) {
		print "\n$dist ($Modules{$dist}{MAINTAINER}):\n" unless ($distname_printed++);
		print "\t$file: core=$vcore, cpan=$vcpan\n";
	    }
	}
    }
}
