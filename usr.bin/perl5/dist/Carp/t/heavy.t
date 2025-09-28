print "1..3\n";

print defined(&Carp::carp) ? "not " : "", "ok 1 # control\n";
require Carp::Heavy;
print defined(&Carp::carp) ? "" : "not ", "ok 2 # carp loaded by Carp::Heavy\n";
eval q{
	print $Carp::Heavy::VERSION eq $Carp::VERSION ? "" : "not ",
		"ok 3 # version numbers match\n";
};

1;
