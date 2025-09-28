BEGIN {
    chdir 't' if -d 't/lib';
    @INC = '../lib' if -d 'lib';
    require Config; import Config;
    if (-d 'lib' and $Config{'extensions'} !~ /\bOS2(::|\/)REXX\b/) {
	print "1..0\n";
	exit 0;
    }
}

use OS2::REXX;

#
# DLL
#
load OS2::REXX "rxu"
  or print "1..0 # skipped: cannot find RXU.DLL\n" and exit;

print "1..19\n";

REXX_call {
  print "ok 1\n";

  #
  # scalar
  #
  tie $s, OS2::REXX, "TEST";
  print "ok 2\n";
  $s = 1;
  print "ok 3\n" if $s eq 1;
  print "not ok 3\n# `$s'\n" unless $s eq 1;
  untie $s;

  #
  # hash
  #

  tie %all, OS2::REXX, "";	# all REXX vars
  print "ok 4\n";

  sub show {
	# show all REXX vars
	print "--@_--\n";
	foreach (keys %all) {
		$v = $all{$_};
		print "$_ => $v\n";
	}
  }

  sub check {
	# check all REXX vars
	my ($test, @arr) = @_;
	my @rx;
	foreach $key (sort keys %all) { push @rx, $key, $all{$key} }
	if ("@rx" eq "@arr") {print "ok $test\n"}
	else { print "not ok $test\n# expect `@arr', got `@rx'\n" }
  }


  tie %h, OS2::REXX, "TEST.";
  print "ok 5\n";
  check(6);

  $h{"one"} = 1;
  check(7, "TEST.one", 1);

  $h{"two"} = 2;
  check(8, "TEST.one", 1, "TEST.two", 2);

  $h{"one"} = "";
  check(9, "TEST.one", "", "TEST.two", 2);
  print "ok 10\n" if exists $h{"one"};
  print "ok 11\n" if exists $h{"two"};

  delete $h{"one"};
  check(12, "TEST.two", 2);
  print "ok 13\n" if not exists $h{"one"};
  print "ok 14\n" if exists $h{"two"};

  OS2::REXX::dropall("TEST.");
  print "ok 15\n";
  check(16);
  print "ok 17\n" if not exists $h{"one"};
  print "ok 18\n" if not exists $h{"two"};

  untie %h;
  print "ok 19";

};
