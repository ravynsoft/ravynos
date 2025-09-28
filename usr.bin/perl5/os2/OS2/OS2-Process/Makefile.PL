use ExtUtils::MakeMaker;

create_constants();		# Make a module

# See lib/ExtUtils/MakeMaker.pm for details of how to influence
# the contents of the Makefile that is written.
WriteMakefile(
    'NAME'	=> 'OS2::Process',
    VERSION_FROM=> 'Process.pm',
    MAN3PODS 	=> {}, 	# Pods will be built by installman.
    'LIBS'	=> [''],   # e.g., '-lm' 
    'DEFINE'	=> '',     # e.g., '-DHAVE_SOMETHING' 
    'INC'	=> '',     # e.g., '-I/usr/include/other' 
    IMPORTS	=> { _16_DosSmSetTitle => 'sesmgr.DOSSMSETTITLE',
		     # _16_Win16SetTitle => 'pmshapi.93',
		   },
);

sub create_constants {
  return if -d 'Process_constants';
  my $src_dir;
  my @try = qw(.. ../.. ../../.. ../../../..);
  for (@try) {
    $src_dir = $_, last if -d "$_/utils" and -r "$_/utils/h2xs";
  }
  warn("Can't find \$PERL_SRC/utils/h2xs in @try, falling back to no constants"),
    return unless defined $src_dir;
  # Can't name it *::Constants, otherwise constants.xs would overwrite it...
  # This produces warnings from PSZ-conversion on WS_* constants.
  system $^X, "-I$src_dir/lib", "$src_dir/utils/h2xs", '-fn', 'OS2::Process::Const',
    '--skip-exporter', '--skip-autoloader', # too large memory overhead
      '--skip-strict', '--skip-warnings', # likewise
	'--skip-ppport',	# will not work without dynaloading.
				# Most useful for OS2::Process:
	  '-M^(HWND|WM|SC|SWP|WC|PROG|QW|EDI|WS|QWS|QWP|QWL|FF|FI|LS|FS|FCF|BS|MS|TBM|CF|CFI|FID|MB|MBID|CF|CFI|SPTR)_',
	    '-F', '-DINCL_NLS -DINCL_BASE -DINCL_PM', # Define more symbols
	      'os2emx.h'	# EMX version of OS/2 API
    and warn("Can't build module with constants, falling back to no constants"),
      return;
  rename 'OS2/Process/Const', 'Process_constants'
    or warn("Error renaming module, falling back to no constants: $!"),
      return;
  return 1;
}
