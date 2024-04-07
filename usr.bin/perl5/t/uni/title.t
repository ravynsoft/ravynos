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
	"Titlecase_Mapping",
        ucfirst                        => sub { ucfirst $_[0] },
	ucfirst_with_appended_null_arg => sub { my $a = ""; ucfirst ($_[0] . $a) }
       );
