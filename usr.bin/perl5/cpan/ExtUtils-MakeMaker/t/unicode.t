# Test problems in Makefile.PL's and hint files.

BEGIN {
    unshift @INC, 't/lib';
}
chdir 't';

use strict;
use warnings;
use ExtUtils::MM;
use MakeMaker::Test::Utils qw(makefile_name make make_run run hash2files);
use Test::More;
use Config;
use File::Path;
use utf8;
BEGIN {
  plan skip_all => 'Need perlio and perl 5.8+.'
    if "$]" < 5.008 or !$Config{useperlio};
  plan skip_all => 'cross-compiling and make not available'
    if !MM->can_run(make()) && $ENV{PERL_CORE} && $Config{'usecrosscompile'};

  plan tests => 8;
}
use TieOut;

my $MM = bless { DIR => ['.'] }, 'MM';

my $DIRNAME = 'Problem-Module';
my %FILES = (
    'Makefile.PL'   => <<'PL_END',
use ExtUtils::MakeMaker;
use utf8;
WriteMakefile(
    NAME          => 'Problem::Module',
    ABSTRACT_FROM => 'lib/Problem/Module.pm',
    AUTHOR        => q{Danijel Tašov},
    EXE_FILES     => [ qw(bin/probscript) ],
    INSTALLMAN1DIR => "some", # even if disabled in $Config{man1dir}
    MAN1EXT       => 1, # set to 0 if man pages disabled
);
PL_END

    'lib/Problem/Module.pm'  => <<'pm_END',
use utf8;

=pod

=encoding utf8

=head1 NAME

Problem::Module - Danijel Tašov's great new module

=cut

1;
pm_END

    'bin/probscript'  => <<'pl_END',
#!/usr/bin/perl
use utf8;

=encoding utf8

=head1 NAME

文档 - Problem script
pl_END
);

hash2files($DIRNAME, \%FILES);
END {
    ok( chdir File::Spec->updir, 'chdir updir' );
    ok( rmtree($DIRNAME), 'teardown' );
}

ok( chdir $DIRNAME, "chdir'd to $DIRNAME" ) ||
  diag("chdir failed: $!");

if ("$]" >= 5.008) {
  eval { require ExtUtils::MakeMaker::Locale; };
  note "ExtUtils::MakeMaker::Locale vars: $ExtUtils::MakeMaker::Locale::ENCODING_LOCALE;$ExtUtils::MakeMaker::Locale::ENCODING_LOCALE_FS;$ExtUtils::MakeMaker::Locale::ENCODING_CONSOLE_IN;$ExtUtils::MakeMaker::Locale::ENCODING_CONSOLE_OUT\n" unless $@;
  note "Locale env vars: " . join(';', map {
    "$_=$ENV{$_}"
  } grep { /LANG|LC/ } keys %ENV) . "\n";
}

# Make sure when Makefile.PL's break, they issue a warning.
# Also make sure Makefile.PL's in subdirs still have '.' in @INC.
{
    my $stdout = tie *STDOUT, 'TieOut' or die;

    my $warning = '';
    local $SIG{__WARN__} = sub { $warning .= join '', @_ };
    $MM->eval_in_subdirs;
    my $warnlines = grep { !/does not map to/ } split "\n", $warning;
    is $warnlines, 0, 'no warning' or diag $warning;

    open my $json_fh, '<:utf8', 'MYMETA.json' or die $!;
    my $json = do { local $/; <$json_fh> };
    close $json_fh;

    no utf8; # leave the data below as bytes and let Encode sort it out
    require Encode;
    my $str = Encode::decode( 'utf8', "Danijel Tašov's" );
    like( $json, qr/$str/, 'utf8 abstract' );

    untie *STDOUT;
}

my $make = make_run();
my $make_out = run($make);
diag $make_out unless is $? >> 8, 0, 'Exit code of make == 0';

my $manfile = File::Spec->catfile(qw(blib man1 probscript.1));
SKIP: {
  skip 'Manpage not generated', 1 unless -f $manfile;
  skip 'Pod::Man >= 2.17 needed', 1 unless do {
    require Pod::Man; $Pod::Man::VERSION >= 2.17;
  };
  open my $man_fh, '<:utf8', $manfile or die "open $manfile: $!";
  my $man = do { local $/; <$man_fh> };
  close $man_fh;

  no utf8; # leave the data below as bytes and let Encode sort it out
  require Encode;
  my $str = Encode::decode( 'utf8', "文档" );
  like( $man, qr/$str/, 'utf8 man-snippet' );
}

$make_out = run("$make realclean");
diag $make_out unless is $? >> 8, 0, 'Exit code of make == 0';

sub makefile_content {
  open my $fh, '<', makefile_name or die;
  return <$fh>;
}
