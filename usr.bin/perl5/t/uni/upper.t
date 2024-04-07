BEGIN {
    chdir 't' if -d 't';
    unless (defined &DynaLoader::boot_DynaLoader) {
      print("1..0 # miniperl: no Unicode::Normalize");
      exit(0);
    }
    require "./uni/case.pl";
}

use feature 'unicode_strings';

is(uc("\x{3B1}\x{345}\x{301}"), "\x{391}\x{301}\x{399}",
                                                   'Verify moves YPOGEGRAMMENI');
fresh_perl_is('use 5.026;m.\U00ÿÿ0000.', "", {}, "[perl #133876]  This caused valgrind and asan errors");

casetest( 2,	# extra tests already run
	"Uppercase_Mapping",
	 uc                        => sub { uc $_[0] },
	 uc_with_appended_null_arg => sub { my $a = ""; uc ($_[0] . $a) }
        );
