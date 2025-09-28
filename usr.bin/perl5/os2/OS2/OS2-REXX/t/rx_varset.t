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

print "1..9\n";

REXX_call {
  OS2::REXX::_set("X" => sqrt(2)) and print "ok 1\n";
  $x = OS2::REXX::_fetch("X") and print "ok 2\n";
  if (abs($x - sqrt(2)) < 5e-15) {
    print "ok 3\n";
  } else {  print "not ok 3\n# sqrt(2) = @{[sqrt(2)]} != `$x'\n" }
  OS2::REXX::_set("Y" => sqrt(3)) and print "ok 4\n";
  $i = 0;
  $n = 4;
  while (($name, $value) = OS2::REXX::_next("")) {
	$i++; $n++;
	if ($i <= 2 and $name eq "Y" ) {
	  if ($value eq sqrt(3)) {
	    print "ok $n\n";
	  } else {
	    print "not ok $n\n# `$name' => `$value'\n" ;
	  }
	} elsif ($i <= 2 and $name eq "X") {
	  print "ok $n\n" if $value eq sqrt(2);
	} else { print "not ok 7\n# name `$name', value `$value'\n" }
  }
  print "ok 7\n" if $i == 2;
  OS2::REXX::_drop("X") and print "ok 8\n";
  $x = OS2::REXX::_fetch("X") or print "ok 9\n";
};
