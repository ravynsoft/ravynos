BEGIN {
    chdir 't' if -d 't';
    unless (defined &DynaLoader::boot_DynaLoader) {
      print("1..0 # miniperl: no Unicode::Normalize");
      exit(0);
    }
    require "./uni/case.pl";
}

use feature 'unicode_strings';

casetest(0, # No extra tests run here,
	"Lowercase_Mapping",
        lc                             => sub { lc $_[0] },
	lc_with_appended_null_arg      => sub { my $a = ""; lc ($_[0] . $a) },
	lcfirst                        => sub { lcfirst $_[0] },
	lcfirst_with_appended_null_arg => sub { my $a = ""; lcfirst ($_[0] . $a) }
       );
