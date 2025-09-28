#!/usr/bin/perl -w

BEGIN {
    unshift @INC, 't/lib';
}

use strict;
use warnings;

use MakeMaker::Test::Utils;
use Config;
use ExtUtils::MM;
use Test::More
    !MM->can_run(make()) && $ENV{PERL_CORE} && $Config{'usecrosscompile'}
    ? (skip_all => "cross-compiling and make not available")
    : (tests => 12);
use File::Spec;
use File::Temp qw[tempdir];
use File::Path;

my $perl = which_perl();
my $make = make_run();
chdir 't';
perl_lib; # sets $ENV{PERL5LIB} relative to t/

my $tmpdir = tempdir( DIR => '../t', CLEANUP => 1 );
use Cwd; my $cwd = getcwd; END { chdir $cwd } # so File::Temp can cleanup
chdir $tmpdir;

my $DIRNAME = 'PL-Module';
my %FILES = (
    'Makefile.PL'   => <<'END',
use ExtUtils::MakeMaker;
# A module for testing PL_FILES
WriteMakefile(
    NAME     => 'PL::Module',
    PL_FILES => { 'single.PL' => 'single.out',
                  'multi.PL'  => [qw(1.out 2.out)],
                  'single-in.PL' => { 'single-in.out' => 'single.in' },
                  'multi-in.PL' => { 'multi-in.out'  => [qw(1.in 2.in)] },
                  'Bar_pm.PL' => '$(INST_LIB)/PL/Bar.pm',
                  'Bar2.pm.PL' => 'Bar2.pm',
    },
);

package MY;
sub init_PM {
  my ($self) = @_;
  $self->SUPER::init_PM;
  $self->{PM}{'Bar2.pm'} = '$(INST_LIBDIR)/Bar2.pm'; # PDL does this in WM args
}
END

    'single.PL'        => _gen_pl_files(),
    'multi.PL'         => _gen_pl_files(),
    'Bar_pm.PL'        => _gen_pm_files(),
    'Bar2.pm.PL'       => _gen_pm_files(),
    'single-in.PL'     => _gen_pm_files(1),
    'multi-in.PL'      => _gen_pm_files(2),
    'single.in'        => '',
    '1.in'             => '',
    '2.in'             => '',
    'lib/PL/Foo.pm' => <<'END',
# Module to load to ensure PL_FILES have blib in @INC.
package PL::Foo;
sub bar { 42 }
1;
END

);

hash2files($DIRNAME, \%FILES);
END {
    ok( chdir File::Spec->updir );
    ok( rmtree($DIRNAME) );
}

ok chdir($DIRNAME);

run(qq{$perl Makefile.PL});
cmp_ok( $?, '==', 0 );

my $make_out = run("$make");
is( $?, 0 ) || diag $make_out;

foreach my $file (qw(
    single.out 1.out 2.out
    single-in.out multi-in.out
    blib/lib/PL/Bar.pm blib/lib/PL/Bar2.pm
)) {
    ok( -e $file, "$file was created" );
}

sub _gen_pl_files {
    my $test = <<'END';
#!/usr/bin/perl -w

# Ensure we have blib in @INC
use PL::Foo;
die unless PL::Foo::bar() == 42;

# Had a bug where PL_FILES weren't sent the file to generate
die "argv empty\n" unless @ARGV;
die "too many in argv: @ARGV\n" unless @ARGV == 1;

my $file = $ARGV[0];
open OUT, ">$file" or die $!;

print OUT "Testing\n";
close OUT
END

    $test =~ s/^\n//;

    return $test;
}

sub _gen_pm_files {
    my $inputs = (shift || 0) + 1;
    my $test = sprintf <<'END', $inputs;
#!/usr/bin/perl -w

# Ensure we do NOT have blib in @INC when building a module
eval { require PL::Foo; };
#die $@ unless $@ =~ m{^Can't locate PL/Foo.pm in \@INC };

# Had a bug where PL_FILES weren't sent the file to generate
die "argv empty\n" unless @ARGV;
die "wrong number in argv: @ARGV\n" unless @ARGV == %d;

my $file = $ARGV[0];
open OUT, ">$file" or die $!;

print OUT "Testing\n";
close OUT
END

    $test =~ s/^\n//;

    return $test;
}
