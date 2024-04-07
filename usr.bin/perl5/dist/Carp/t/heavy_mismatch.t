print "1..1\n";

$INC{"Carp.pm"} = "<faked>";
$Carp::VERSION = "0.90";
eval { require Carp::Heavy; };
print $@ =~ /\AVersion mismatch between Carp 0\.90 \(<faked>\) and Carp::Heavy [0-9._]+ \(.+\)\.  Did you alter \@INC after Carp was loaded\?\n/ ? "" : "not ", "ok 1\n";

1;
