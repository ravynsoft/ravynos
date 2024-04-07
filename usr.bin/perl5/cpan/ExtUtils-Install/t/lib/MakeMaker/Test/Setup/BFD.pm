package MakeMaker::Test::Setup::BFD;
use strict;

our @ISA = qw(Exporter);
require Exporter;
our @EXPORT = qw(setup_recurs teardown_recurs);

use File::Path;
use File::Basename;
use MakeMaker::Test::Utils;

my %Files = (
             'Big-Dummy/lib/Big/Dummy.pm'     => <<'END',
package Big::Dummy;

$VERSION = 0.02;

=head1 NAME

Big::Dummy - Try "our" hot dog's

=cut

1;
END

             'Big-Dummy/Makefile.PL'          => <<'END',
use ExtUtils::MakeMaker;

# This will interfere with the PREREQ_PRINT tests.
printf "Current package is: %s\n", __PACKAGE__ unless "@ARGV" =~ /PREREQ/;

WriteMakefile(
    NAME          => 'Big::Dummy',
    VERSION_FROM  => 'lib/Big/Dummy.pm',
    EXE_FILES     => [qw(bin/program)],
    PREREQ_PM     => { strict => 0 },
    BUILD_REQUIRES => { warnings => 0 },
    ABSTRACT_FROM => 'lib/Big/Dummy.pm',
    AUTHOR        => 'Michael G Schwern <schwern@pobox.com>',
);
END

             'Big-Dummy/bin/program'          => <<'END',
#!/usr/bin/perl -w

=head1 NAME

program - this is a program

=cut

1;
END

             'Big-Dummy/t/compile.t'          => <<'END',
print "1..2\n";

print eval "use Big::Dummy; 1;" ? "ok 1\n" : "not ok 1\n";
print "ok 2 - TEST_VERBOSE\n";
END

             'Big-Dummy/Liar/t/sanity.t'      => <<'END',
print "1..3\n";

print eval "use Big::Dummy; 1;" ? "ok 1\n" : "not ok 1\n";
print eval "use Big::Liar; 1;" ? "ok 2\n" : "not ok 2\n";
print "ok 3 - TEST_VERBOSE\n";
END

             'Big-Dummy/Liar/lib/Big/Liar.pm' => <<'END',
package Big::Liar;

$VERSION = 0.01;

1;
END

             'Big-Dummy/Liar/Makefile.PL'     => <<'END',
use ExtUtils::MakeMaker;

my $mm = WriteMakefile(
              NAME => 'Big::Liar',
              VERSION_FROM => 'lib/Big/Liar.pm',
              _KEEP_AFTER_FLUSH => 1
             );

print "Big::Liar's vars\n";
foreach my $key (qw(INST_LIB INST_ARCHLIB)) {
    print "$key = $mm->{$key}\n";
}
END

             'Big-Dummy/lib/Dummy/Split.pm'     => <<'END',
package Dummy::Split;
$VERSION = 0.02;
use AutoLoader 'AUTOLOAD';

__END__

sub split { print "split\n"; }

1;
END

            );

my $tmpdir;

# if given args, those are inserted as components in resulting path, eg:
# setup_recurs('dir') means instead of creating Big-Dummy/*, dir/Big-Dummy/*
sub setup_recurs {
    my @chrs = ( "A" .. "Z", 0 .. 9 );
    # annoyingly we cant use File::Temp here as it drags in XS code
    # and we run under blocks to prevent XS code loads. This is a minimal
    # patch to fix the issue.
    $tmpdir = join "", "./temp-$$-", map { $chrs[rand(@chrs)] } 1..8;
    mkdir($tmpdir) or die "Failed to create '$tmpdir': $!";
    chdir($tmpdir) or die "Failed to chdir '$tmpdir': $!";
    foreach my $file (sort keys %Files) {
        my $text = $Files{$file};
        # Convert to a relative, native file path.
        $file = File::Spec->catfile(File::Spec->curdir, @_, split m{\/}, $file);
        $file = File::Spec->rel2abs($file);

        my $dir = dirname($file);
        mkpath $dir;
        open(FILE, ">$file") || die "Can't create $file: $!";
        print FILE $text;
        close FILE;

        # ensure file at least 1 second old for makes that assume
        # files with the same time are out of date.
        my $time = calibrate_mtime();
        utime $time, $time - 1, $file;
    }

    return 1;
}

sub teardown_recurs {
    foreach my $file (keys %Files) {
        my $dir = dirname($file);
        if( -e $dir ) {
            rmtree($dir) or next;
        }
    }
    chdir("..");
    rmtree($tmpdir);
    return 1;
}


1;
