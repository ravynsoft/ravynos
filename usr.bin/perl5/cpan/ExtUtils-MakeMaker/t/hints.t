#!/usr/bin/perl -w

use strict;
use warnings;

use lib 't/lib';

use File::Temp qw[tempdir];
my $tmpdir = tempdir( DIR => 't', CLEANUP => 1 );
use Cwd; my $cwd = getcwd; END { chdir $cwd } # so File::Temp can cleanup
chdir $tmpdir;
use File::Spec;

use Test::More tests => 3;

# Having the CWD in @INC masked a bug in finding hint files
my $curdir = File::Spec->curdir;
@INC = grep { $_ ne $curdir && $_ ne '.' } @INC;

use ExtUtils::MakeMaker;

# Make a hints directory for testing
mkdir('hints', 0777);
(my $os = $^O) =~ s/\./_/g;
my $Hint_File = File::Spec->catfile('hints', "$os.pl");


my $mm = bless {}, 'ExtUtils::MakeMaker';

# Write a hints file for testing
{
    open my $hint_fh, ">", $Hint_File || die "Can't write dummy hints file $Hint_File: $!";
    print $hint_fh <<'CLOO';
$self->{CCFLAGS} = 'basset hounds got long ears';
CLOO
}

# Test our hint file is detected
{
    my $stderr = '';
    local $SIG{__WARN__} = sub { $stderr .= join '', @_ };

    $mm->check_hints;
    is( $mm->{CCFLAGS}, 'basset hounds got long ears' );
    is( $stderr, "" );
}


# Test a hint file which dies
{
    open my $hint_fh, ">", $Hint_File || die "Can't write dummy hints file $Hint_File: $!";
    print $hint_fh <<'CLOO';
die "Argh!\n";
CLOO
}


# Test the hint file which produces errors
{
    my $stderr = '';
    local $SIG{__WARN__} = sub { $stderr .= join '', @_ };

    $mm->check_hints;
    is( $stderr, <<OUT, 'hint files produce errors' );
Argh!
OUT
}

END {
    use File::Path;
    rmtree ['hints'];
}
