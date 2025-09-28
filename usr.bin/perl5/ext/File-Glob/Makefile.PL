use ExtUtils::MakeMaker;
use ExtUtils::Constant 0.23 'WriteConstants';
WriteMakefile(
    NAME		=> 'File::Glob',
    VERSION_FROM	=> 'Glob.pm',
    OBJECT		=> 'bsd_glob$(OBJ_EXT) Glob$(OBJ_EXT)',
    realclean => {FILES=> 'const-c.inc const-xs.inc'},

## uncomment for glob debugging (will cause make test to fail)
#   DEFINE		=> '-DGLOB_DEBUG',
#   OPTIMIZE		=> '-g',
);
use Config;
sub MY::cflags {
  package MY;
  my $inherited = shift->SUPER::cflags(@_);
  if ($Config::Config{archname} =~ /^aix/ and 
      $Config::Config{use64bitall} eq 'define') {
    $inherited =~ s/\s-O\d?//m;
  }
  $inherited;
}

WriteConstants(
    PROXYSUBS => {autoload => 1},
    NAME => 'File::Glob',
    NAMES => [qw(GLOB_ABEND GLOB_ALPHASORT GLOB_ALTDIRFUNC GLOB_BRACE GLOB_ERR
                 GLOB_LIMIT GLOB_MARK GLOB_NOCASE GLOB_NOCHECK GLOB_NOMAGIC
                 GLOB_NOSORT GLOB_NOSPACE GLOB_QUOTE GLOB_TILDE),
	     {name => 'GLOB_CSH',
	      value => 'GLOB_BRACE|GLOB_NOMAGIC|GLOB_QUOTE|GLOB_TILDE|GLOB_ALPHASORT',
	      macro => 1},
	     ],
    BREAKOUT_AT => 8,
);
