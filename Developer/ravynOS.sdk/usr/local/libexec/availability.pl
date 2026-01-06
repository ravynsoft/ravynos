#!/usr/bin/perl

use strict;
use warnings;
use Getopt::Long;
use List::Util qw(pairs);
use List::Util qw(pairgrep);
use List::Util qw(pairvalues);

my @versions = (
    "macos",    "10.0",
    "macos",    "10.1",
    "macos",    "10.2",
    "macos",    "10.3",
    "macos",    "10.4",
    "macos",    "10.5",
    "macos",    "10.6",
    "macos",    "10.7",
    "ios",      "2.0",
    "ios",      "2.1",
    "ios",      "2.2",
    "ios",      "3.0",
    "ios",      "3.1",
    "ios",      "3.2",
    "ios",      "4.0",
    "ios",      "4.1",
    "ios",      "4.2",
    "ios",      "4.3",
    "ios",      "5.0",
    "os_set",   "fall_2011",
    "ios",      "5.1",
    "macos",    "10.8",
    "ios",      "6.0",
    "os_set",   "fall_2012",
    "macos",    "10.9",
    "ios",      "6.1",
    "ios",      "7.0",
    "os_set",   "fall_2013",
    "macos",    "10.10",
    "ios",      "7.1",
    "ios",      "8.0",
    "os_set",   "fall_2014",
    "watchos",  "1.0",
    "macos",    "10.10.2",
    "macos",    "10.10.3",
    "macos",    "10.11",
    "ios",      "8.1",
    "ios",      "8.2",
    "ios",      "8.3",
    "ios",      "8.4",
    "ios",      "9.0",
    "tvos",     "9.0",
    "watchos",  "2.0",
    "os_set",   "fall_2015",
    "macos",    "10.11.2",
    "macos",    "10.11.3",
    "macos",    "10.11.4",
    "macos",    "10.12",
    "ios",      "9.1",
    "ios",      "9.2",
    "ios",      "9.3",
    "ios",      "10.0",
    "watchos",  "2.1",
    "watchos",  "2.2",
    "tvos",     "9.1",
    "tvos",     "9.2",
    "tvos",     "10.0",
    "watchos",  "3.0",
    "os_set",   "fall_2016",
    "macos",    "10.12.1",
    "macos",    "10.12.2",
    "macos",    "10.12.4",
    "ios",      "10.1",
    "ios",      "10.2",
    "ios",      "10.3",
    "tvos",     "10.0.1",
    "tvos",     "10.1",
    "tvos",     "10.2",
    "watchos",  "3.1",
    "watchos",  "3.1.1",
    "watchos",  "3.2",
    "macos",    "10.13",
    "ios",      "11.0",
    "tvos",     "11.0",
    "watchos",  "4.0",
    "bridgeos", "2.0",
    "os_set",   "fall_2017",
    "macos",    "10.13.1",
    "macos",    "10.13.2",
    "ios",      "11.1",
    "ios",      "11.2",
    "tvos",     "11.1",
    "tvos",     "11.2",
    "watchos",  "4.1",
    "watchos",  "4.2",
    "os_set",   "winter_2017",
    "macos",    "10.13.4",
    "ios",      "11.3",
    "ios",      "11.4",
    "tvos",     "11.3",
    "tvos",     "11.4",
    "watchos",  "4.3",
    "macos",    "10.14",
    "ios",      "12.0",
    "tvos",     "12.0",
    "watchos",  "5.0",
    "bridgeos", "3.0",
    "os_set",   "fall_2018",
    "macos",    "10.14.1",
    "ios",      "12.1",
    "tvos",     "12.1",
    "watchos",  "5.1",
    "bridgeos", "3.1",
    "os_set",   "late_fall_2018",
    "macos",    "10.14.4",
    "ios",      "12.2",
    "tvos",     "12.2",
    "watchos",  "5.2",
    "bridgeos", "3.4",
    "os_set",   "spring_2019",
    "ios",      "12.3",
    "tvos",     "12.3",
    "macos",    "10.14.5",
    "os_set",   "summer_2019",
    "macos",    "10.14.6",
    "ios",      "12.4",
    "tvos",     "12.4",
    "watchos",  "5.3",
    "os_set",   "late_summer_2019",
    "macos",    "10.15",
    "ios",      "13.0",
    "tvos",     "13.0",
    "watchos",  "6.0",
    "bridgeos", "4.0",
    "os_set",   "fall_2019",
    "ios",      "13.1",
    "os_set",   "autumn_2019",
    "macos",    "10.15.1",
    "ios",      "13.2",
    "tvos",     "13.2",
    "watchos",  "6.1",
    "bridgeos", "4.1",
    "os_set",   "late_fall_2019",
    "ios",      "13.3",
    "tvos",     "13.3",
    "os_set",   "winter_2019",
    "ios",      "13.4",
    "tvos",     "13.4",
    "watchos",  "6.2",
    "os_set",   "spring_2020",
    "ios",      "13.5",
    "os_set",   "late_spring_2020",
    "ios",      "13.6",
    "os_set",   "summer_2020",
    "ios",      "13.7",
    "os_set",   "late_summer_2020",
    "macos",    "10.16",
    "ios",      "14.0",
    "tvos",     "14.0",
    "watchos",  "7.0",
    "bridgeos", "5.0",
    "os_set",   "fall_2020",
    "ios",      "14.1",
    "tvos",     "14.1",
    "watchos",  "7.1",
    "ios",      "14.2",
    "tvos",     "14.2",
    "os_set",   "late_fall_2020",
    "macos",    "11.00",
);

my @macosx_versions = pairvalues pairgrep { $a eq "macos" } @versions;
my @ios_versions = pairvalues pairgrep { $a eq "ios" } @versions;
my @appletvos_versions = pairvalues pairgrep { $a eq "tvos" } @versions;
my @watchos_versions = pairvalues pairgrep { $a eq "watchos" } @versions;
my @bridgeos_versions = pairvalues pairgrep { $a eq "bridgeos" } @versions;

my $m;
my $i;
my $a;
my $w;
my $b;
my $s;
GetOptions('macosx' => \$m, 'osx' => \$m, 'ios' => \$i, 'appletvos' => \$a, 'watchos' => \$w, 'bridgeos' => \$b, 'sets' => \$s);

sub print_sets {
    my @version_pairs = pairs @versions;
    my %oses;
    print "---\n";
    foreach my $version (@version_pairs) {
        if ($version->[0] eq "os_set") {
            print $version->[1] . ":\n";
            keys %oses;
            while(my($k, $v) = each %oses) {
                printf "\  " . $k . ":\  \"" . $v . "\"\n";
            }
        } else {
            $oses{$version->[0]} = $version->[1];
        }
    }
}

if ($m) {
  print join(" ", @macosx_versions) . "\n";
} elsif ($i) {
  print join(" ", @ios_versions) . "\n";
} elsif ($a) {
  print join(" ", @appletvos_versions) . "\n";
} elsif ($w) {
  print join(" ", @watchos_versions) . "\n";
} elsif ($b) {
  print join(" ", @bridgeos_versions) . "\n";
} elsif ($s) {
    print_sets();
} else {
  print "usage: $0 --macosx|--osx|--ios|--appletvos|--watchos|--bridgeos|--sets\n";
}

