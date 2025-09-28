#!perl -w
use strict;
require './regen/regen_lib.pl';
require './Porting/pod_lib.pl';
our ($TAP, $Verbose);

# For processing later
my @ext;
# Lookup hash of all directories in lib/ in a clean distribution
my %libdirs;

open my $fh, '<', 'MANIFEST'
    or die "Can't open MANIFEST: $!";

while (<$fh>) {
    if (m<^((?:cpan|dist|ext)/[^/]+/              # In an extension directory
           (?!t/|private/|corpus/|demo/|testdir/) # but not a test or similar
           \S+                                    # filename characters
           (?:\.pm|\.pod|_pm\.PL|_pod\.PL|\.yml)) # useful ending
           (?:\s|$)                               # whitespace or end of line
          >x) {
        push @ext, $1;
    } elsif (m!^lib/([^ \t\n]+)/[^/ \t\n]+!) {
        # All we are interested in are shipped directories in lib/
        # leafnames (and package names) are actually irrelevant.
        my $dirs = $1;
        do {
            # lib/Pod/t is in MANIFEST, but lib/Pod is not. Rather than
            # special-casing this, generalise the code to ensure that all
            # parent directories of anything add are also added:
            ++$libdirs{$dirs}
        } while ($dirs =~ s!/.*!!);
    }
}

close $fh
    or die "Can't close MANIFEST: $!";

# Lines we need in lib/.gitignore
my %ignore;
# Directories that the Makfiles should remove
# With a special case already :-(
my %rmdir_s = my %rmdir = ('Unicode/Collate/Locale' => 1);

FILE:
foreach my $file (@ext) {
    my ($extname, $path) = $file =~ m!^(?:cpan|dist|ext)/([^/]+)/(.*)!
        or die "Can't parse '$file'";

    if ($path =~ /\.yml$/) {
        next unless $path =~ s!^lib/!!;
    } elsif ($path =~ /\.pod$/) {
        unless ($path =~ s!^lib/!!) {
            # ExtUtils::MakeMaker will install it to a path based on the
            # extension name:
            if ($extname =~ s!-[^-]+$!!) {
                $extname =~ tr!-!/!;
                $path = "$extname/$path";
            }
        }
    } elsif ($extname eq 'Unicode-Collate'  # Trust the package lines
             || $extname eq 'Encode'        # Trust the package lines
             || $path eq 'win32/Win32.pm'   # Trust the package line
             || ($path !~ tr!/!!            # No path
                 && $path ne 'DB_File.pm'   # ... but has multiple package lines
                )) {
        # Too many special cases to encode, so just open the file and figure it
        # out:
        my $package;
        open my $fh, '<', $file
            or die "Can't open $file: $!";
        while (<$fh>) {
            if (/^\s*package\s+([A-Za-z0-9_:]+)/) {
                $package = $1;
                last;
            }
        }
        close $fh
            or die "Can't close $file: $!";
        die "Can't locate package statement in $file"
            unless defined $package;
        $package =~ s!::!/!g;
        $path = "$package.pm";
    } else {
        if ($path =~ s/\.PL$//) {
            # .PL files generate other files. By convention the output filename
            # has the .PL stripped, and any preceding _ changed to ., to comply
            # with historical VMS filename rules that only permit one .
            $path =~ s!_([^_/]+)$!.$1!;
        }
        $path =~ s!^lib/!!;
    }
    my @parts = split '/', $path;
    my $prefix = shift @parts;
    while (@parts) {
        if (!$libdirs{$prefix}) {
            # It is a directory that we will create. Ignore everything in it:
            ++$ignore{"/$prefix/"};
            ++$rmdir{$prefix};
            ++$rmdir_s{$prefix};
            pop @parts;
            while (@parts) {
                $prefix .= '/' . shift @parts;
                ++$rmdir{$prefix};
            }
            next FILE;
        }
        $prefix .= '/' . shift @parts;
        # If we've just shifted the leafname back onto $prefix, then @parts is
        # empty, so we should terminate this loop.
    }
    # We are creating a file in an existing directory. We must ignore the file
    # explicitly:
    ++$ignore{"/$path"};
}

sub edit_makefile_SH {
    my ($desc, $contents) = @_;
    my $start_re = qr/(\trm -f so_locations[^\n]+)/;
    my ($start) = $contents =~ $start_re;
    $contents = verify_contiguous($desc, $contents,
                                  qr/$start_re\n(?:\t-rmdir [^\n]+\n)+/sm,
                                  'lib directory rmdir rules');
    # Reverse sort ensures that any subdirectories are deleted first.
    # The extensions themselves delete files with the MakeMaker generated clean
    # targets.
    $contents =~ s{\0}
                  {"$start\n"
                   . wrap(79, "\t-rmdir ", "\t-rmdir ",
                          map {"lib/$_"} reverse sort keys %rmdir)
                   . "\n"}e;
    $contents;
}

sub edit_win32_makefile {
    my ($desc, $contents) = @_;
    my $start = "\t-del /f *.def *.map";
    my $start_re = quotemeta($start);
    $contents = verify_contiguous($desc, $contents,
                                  qr!$start_re\n(?:\t-if exist (\$\(LIBDIR\)\\\S+) rmdir /s /q \1\n)+!sm,
                                  'Win32 lib directory rmdir rules');
    # Win32 is (currently) using rmdir /s /q which deletes recursively
    # (seems to be analogous to rm -r) so we don't explicitly list
    # subdirectories to delete, and don't need to ensure that subdirectories are
    # deleted before their parents.
    # Might be able to rely on MakeMaker generated clean targets to clean
    # everything, but not in a position to test this.
    my $lines = join '', map {
        tr!/!\\!;
        "\t-if exist \$(LIBDIR)\\$_ rmdir /s /q \$(LIBDIR)\\$_\n"
    } sort {lc $a cmp lc $b} keys %rmdir_s;
    $contents =~ s/\0/$start\n$lines/;
    $contents;
}

process('Makefile.SH', 'Makefile.SH', \&edit_makefile_SH, $TAP && '', $Verbose);
foreach ('win32/Makefile', 'win32/GNUmakefile') {
    process($_, $_, \&edit_win32_makefile, $TAP && '', $Verbose);
}

# This must come last as it can exit early:
if ($TAP && !-d '.git' && !-f 'lib/.gitignore') {
    print "ok # skip not being run from a git checkout, hence no lib/.gitignore\n";
    exit 0;
}

if ($ENV{'PERL_BUILD_PACKAGING'}) {
    print "ok # skip explicitly disabled git tests by PERL_BUILD_PACKAGING\n";
    exit 0;
}

$fh = open_new('lib/.gitignore', '>',
               { by => $0,
                 from => 'MANIFEST and parsing files in cpan/ dist/ and ext/'});

print $fh <<"EOT";
# If this generated file has problems, it may be simpler to add more special
# cases to the top level .gitignore than to code one-off logic into the
# generation script $0

EOT

print $fh "$_\n" foreach sort keys %ignore;

read_only_bottom_close_and_rename($fh);
