use XS::APItest;
use Test::More tests => 1;
use Data::Dumper;
my $warnings = "";
$SIG{__WARN__} = sub { $warnings .= $_[0]; };

warn "Before test_mortal_destructor_sv\n";
test_mortal_destructor_sv(sub { warn "in perl callback: ", $_[0],"\n" }, {});
warn "After test_mortal_destructor_sv\n";

warn "Before test_mortal_destructor_av\n";
test_mortal_destructor_av(sub { warn "in perl callback: @_\n" }, ["a","b","c"]);
warn "After test_mortal_destructor_av\n";

warn "Before test_mortal_destructor_x\n";
test_mortal_svfunc_x("this is an argument");
warn "After test_mortal_destructor_x\n";

$warnings=~s/0x[A-Fa-f0-9]+/0xDEADBEEF/g;
is($warnings, <<'EXPECT');
Before test_mortal_destructor_sv
in perl callback: HASH(0xDEADBEEF)
After test_mortal_destructor_sv
Before test_mortal_destructor_av
in perl callback: a b c
After test_mortal_destructor_av
Before test_mortal_destructor_x
In destruct_test: this is an argument
After test_mortal_destructor_x
EXPECT
