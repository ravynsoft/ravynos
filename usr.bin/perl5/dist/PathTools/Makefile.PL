
# See https://rt.cpan.org/Public/Bug/Display.html?id=4681
# and https://rt.perl.org/Ticket/Display.html?id=125603
# When installing a newer Cwd on a system with an existing Cwd,
# under some circumstances the old Cwd.pm and the new Cwd.xs could
# get mixed up and SEGVs ensue.

BEGIN { @INC = grep { $_ ne "blib/arch" and $_ ne "blib/lib" } @INC }

require 5.005;
use ExtUtils::MakeMaker;

my @extra;
push @extra, 'LICENSE' => 'perl_5'
    unless $ExtUtils::MakeMaker::VERSION < 6.31;
push @extra, 'META_MERGE' => {
        resources => {
            repository  => 'git://perl5.git.perl.org/perl.git',
            bugtracker  => 'https://github.com/Perl/perl5/issues',
            homepage    => "http://dev.perl.org/",
            license     => [ 'http://dev.perl.org/licenses/' ],
        },
    } unless $ExtUtils::MakeMaker::VERSION < 6.46;

WriteMakefile
(
          'DISTNAME' => 'PathTools',
          'NAME' => 'Cwd',
          'VERSION_FROM' => 'Cwd.pm',
          'ABSTRACT' => 'Tools for working with directory and file names',
          'AUTHOR' => 'Perl 5 Porters',
          'DEFINE' => "-DDOUBLE_SLASHES_SPECIAL=@{[$^O eq q(qnx) || $^O eq q(nto) ? 1 : 0]}",
          'PREREQ_PM' => {
                           'Carp' => '0',
                           'File::Basename' => '0',
                           'Scalar::Util' => '0',
                           # done_testing() is used in dist/Cwd/t/Spec.t
                           'Test::More' => 0.88,
                         },
          ($] > 5.011) ? () : ( INSTALLDIRS => 'perl' ), # CPAN sourced versions should now install to site
          'EXE_FILES' => [],
          'PL_FILES' => {},
          @extra,
        )
;
