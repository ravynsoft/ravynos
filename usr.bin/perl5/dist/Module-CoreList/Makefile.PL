use ExtUtils::MakeMaker;

use strict;
use warnings;

my @extra;
push @extra, 'INSTALLDIRS' => 'perl' if $] >= 5.008009 and $] < 5.012;

push @extra, 'META_MERGE' => {
        resources => {
            repository => 'https://github.com/Perl/perl5.git',
            bugtracker => 'https://github.com/Perl/perl5/issues',
            homepage   => "http://dev.perl.org/",
        },
    } unless $ExtUtils::MakeMaker::VERSION < 6.46;


WriteMakefile
(
    'NAME' => 'Module::CoreList',
    'VERSION_FROM' => 'lib/Module/CoreList.pm',
    'ABSTRACT_FROM' => 'lib/Module/CoreList.pod',
    'PREREQ_PM' => {
        'Test::More' => '0',
        'List::Util' => 0,
        'version' => 0.88,
    },
    'EXE_FILES' => [ _scripts() ],
    'INSTALLDIRS' => ($] < 5.011 ? 'perl' : 'site'),
    'PL_FILES' => {},
    LICENSE => 'perl',
    @extra,
)
;

sub _scripts {
 my $scripts = 'corelist';
 if ( $] >= 5.008009 and !$ENV{PERL_CORE} ) {
   require Config;
   my $version = sprintf("%vd",$^V);
   if ( $Config::Config{versiononly} and
      $Config::Config{startperl} =~ /\Q$version\E$/ ) {
      require File::Copy;
      File::Copy::copy( 'corelist', "corelist$version" );
      $scripts = "corelist$version";
    }
 }
 return $scripts;
}
