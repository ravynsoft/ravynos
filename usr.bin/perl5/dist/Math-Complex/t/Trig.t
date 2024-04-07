#!./perl

#
# Regression tests for the Math::Trig package
#
# The tests here are quite modest as the Math::Complex tests exercise
# these interfaces quite vigorously.
# 
# -- Jarkko Hietaniemi, April 1997

use strict;
use warnings;
use Test::More tests => 157;

use Math::Trig 1.18;
use Math::Trig 1.18 qw(:pi Inf);

our $vax_float = (pack("d",1) =~ /^[\x80\x10]\x40/);
our $has_inf   = !$vax_float;

my $pip2 = pi / 2;

use strict;

our($x, $y, $z);

my $eps = 1e-11;

if ($^O eq 'unicos') { # See lib/Math/Complex.pm and t/lib/complex.t.
    $eps = 1e-10;
}

sub near {
    my $e = defined $_[2] ? $_[2] : $eps;
    my $d = $_[1] ? abs($_[0]/$_[1] - 1) : abs($_[0]);
    print "# near? $_[0] $_[1] : $d : $e\n";
    $_[1] ? ($d < $e) : abs($_[0]) < $e;
}

print "# Sanity checks\n";

ok(near(sin(1), 0.841470984807897));
ok(near(cos(1), 0.54030230586814));
ok(near(tan(1), 1.5574077246549));

ok(near(sec(1), 1.85081571768093));
ok(near(csc(1), 1.18839510577812));
ok(near(cot(1), 0.642092615934331));

ok(near(asin(1), 1.5707963267949));
ok(near(acos(1), 0));
ok(near(atan(1), 0.785398163397448));

ok(near(asec(1), 0));
ok(near(acsc(1), 1.5707963267949));
ok(near(acot(1), 0.785398163397448));

ok(near(sinh(1), 1.1752011936438));
ok(near(cosh(1), 1.54308063481524));
ok(near(tanh(1), 0.761594155955765));

ok(near(sech(1), 0.648054273663885));
ok(near(csch(1), 0.850918128239322));
ok(near(coth(1), 1.31303528549933));

ok(near(asinh(1), 0.881373587019543));
ok(near(acosh(1), 0));
ok(near(atanh(0.9), 1.47221948958322)); # atanh(1.0) would be an error.

ok(near(asech(0.9), 0.467145308103262));
ok(near(acsch(2), 0.481211825059603));
ok(near(acoth(2), 0.549306144334055));

print "# Basics\n";

$x = 0.9;
ok(near(tan($x), sin($x) / cos($x)));

ok(near(sinh(2), 3.62686040784702));

ok(near(acsch(0.1), 2.99822295029797));

$x = asin(2);
is(ref $x, 'Math::Complex');

# avoid using Math::Complex here
$x =~ /^([^-]+)(-[^i]+)i$/;
($y, $z) = ($1, $2);
ok(near($y,  1.5707963267949));
ok(near($z, -1.31695789692482));

ok(near(deg2rad(90), pi/2));

ok(near(rad2deg(pi), 180));

use Math::Trig ':radial';

{
    my ($r,$t,$z) = cartesian_to_cylindrical(1,1,1);

    ok(near($r, sqrt(2)));
    ok(near($t, deg2rad(45)));
    ok(near($z, 1));

    ($x,$y,$z) = cylindrical_to_cartesian($r, $t, $z);

    ok(near($x, 1));
    ok(near($y, 1));
    ok(near($z, 1));

    ($r,$t,$z) = cartesian_to_cylindrical(1,1,0);

    ok(near($r, sqrt(2)));
    ok(near($t, deg2rad(45)));
    ok(near($z, 0));

    ($x,$y,$z) = cylindrical_to_cartesian($r, $t, $z);

    ok(near($x, 1));
    ok(near($y, 1));
    ok(near($z, 0));
}

{
    my ($r,$t,$f) = cartesian_to_spherical(1,1,1);

    ok(near($r, sqrt(3)));
    ok(near($t, deg2rad(45)));
    ok(near($f, atan2(sqrt(2), 1)));

    ($x,$y,$z) = spherical_to_cartesian($r, $t, $f);

    ok(near($x, 1));
    ok(near($y, 1));
    ok(near($z, 1));
       
    ($r,$t,$f) = cartesian_to_spherical(1,1,0);

    ok(near($r, sqrt(2)));
    ok(near($t, deg2rad(45)));
    ok(near($f, deg2rad(90)));

    ($x,$y,$z) = spherical_to_cartesian($r, $t, $f);

    ok(near($x, 1));
    ok(near($y, 1));
    ok(near($z, 0));
}

{
    my ($r,$t,$z) = cylindrical_to_spherical(spherical_to_cylindrical(1,1,1));

    ok(near($r, 1));
    ok(near($t, 1));
    ok(near($z, 1));

    ($r,$t,$z) = spherical_to_cylindrical(cylindrical_to_spherical(1,1,1));

    ok(near($r, 1));
    ok(near($t, 1));
    ok(near($z, 1));
}

{
    use Math::Trig 'great_circle_distance';

    ok(near(great_circle_distance(0, 0, 0, pi/2), pi/2));

    ok(near(great_circle_distance(0, 0, pi, pi), pi));

    # London to Tokyo.
    my @L = (deg2rad(-0.5),  deg2rad(90 - 51.3));
    my @T = (deg2rad(139.8), deg2rad(90 - 35.7));

    my $km = great_circle_distance(@L, @T, 6378);

    ok(near($km, 9605.26637021388));
}

{
    my $R2D = 57.295779513082320876798154814169;

    sub frac { $_[0] - int($_[0]) }

    my $lotta_radians = deg2rad(1E+20, 1);
    ok(near($lotta_radians,  1E+20/$R2D));

    my $negat_degrees = rad2deg(-1E20, 1);
    ok(near($negat_degrees, -1E+20*$R2D));

    my $posit_degrees = rad2deg(-10000, 1);
    ok(near($posit_degrees, -10000*$R2D));
}

{
    use Math::Trig 'great_circle_direction';

    ok(near(great_circle_direction(0, 0, 0, pi/2), pi));

# Retired test: Relies on atan2(0, 0), which is not portable.
#	ok(near(great_circle_direction(0, 0, pi, pi), -pi()/2));

    my @London  = (deg2rad(  -0.167), deg2rad(90 - 51.3));
    my @Tokyo   = (deg2rad( 139.5),   deg2rad(90 - 35.7));
    my @Berlin  = (deg2rad ( 13.417), deg2rad(90 - 52.533));
    my @Paris   = (deg2rad (  2.333), deg2rad(90 - 48.867));

    ok(near(rad2deg(great_circle_direction(@London, @Tokyo)),
	    31.791945393073));

    ok(near(rad2deg(great_circle_direction(@Tokyo, @London)),
	    336.069766430326));

    ok(near(rad2deg(great_circle_direction(@Berlin, @Paris)),
	    246.800348034667));
    
    ok(near(rad2deg(great_circle_direction(@Paris, @Berlin)),
	    58.2079877553156));

    use Math::Trig 'great_circle_bearing';

    ok(near(rad2deg(great_circle_bearing(@Paris, @Berlin)),
	    58.2079877553156));

    use Math::Trig 'great_circle_waypoint';
    use Math::Trig 'great_circle_midpoint';

    my ($lon, $lat);

    ($lon, $lat) = great_circle_waypoint(@London, @Tokyo, 0.0);

    ok(near($lon, $London[0]));

    ok(near($lat, $London[1]));

    ($lon, $lat) = great_circle_waypoint(@London, @Tokyo, 1.0);

    ok(near($lon, $Tokyo[0]));

    ok(near($lat, $Tokyo[1]));

    ($lon, $lat) = great_circle_waypoint(@London, @Tokyo, 0.5);

    ok(near($lon, 1.55609593577679)); # 89.16 E

    ok(near($lat, 0.36783532946162)); # 68.93 N

    ($lon, $lat) = great_circle_midpoint(@London, @Tokyo);

    ok(near($lon, 1.55609593577679)); # 89.16 E

    ok(near($lat, 0.367835329461615)); # 68.93 N

    ($lon, $lat) = great_circle_waypoint(@London, @Tokyo, 0.25);

    ok(near($lon, 0.516073562850837)); # 29.57 E

    ok(near($lat, 0.400231313403387)); # 67.07 N

    ($lon, $lat) = great_circle_waypoint(@London, @Tokyo, 0.75);

    ok(near($lon, 2.17494903805952)); # 124.62 E

    ok(near($lat, 0.617809294053591)); # 54.60 N

    use Math::Trig 'great_circle_destination';

    my $dir1 = great_circle_direction(@London, @Tokyo);
    my $dst1 = great_circle_distance(@London,  @Tokyo);

    ($lon, $lat) = great_circle_destination(@London, $dir1, $dst1);

    ok(near($lon, $Tokyo[0]));

    ok(near($lat, $pip2 - $Tokyo[1]));

    my $dir2 = great_circle_direction(@Tokyo, @London);
    my $dst2 = great_circle_distance(@Tokyo,  @London);

    ($lon, $lat) = great_circle_destination(@Tokyo, $dir2, $dst2);

    ok(near($lon, $London[0]));

    ok(near($lat, $pip2 - $London[1]));

    my $dir3 = (great_circle_destination(@London, $dir1, $dst1))[2];

    ok(near($dir3, 2.69379263839118)); # about 154.343 deg

    my $dir4 = (great_circle_destination(@Tokyo,  $dir2, $dst2))[2];

    ok(near($dir4, 3.6993902625701)); # about 211.959 deg

    ok(near($dst1, $dst2));
}

SKIP: {
# With netbsd-vax (or any vax) there is neither Inf, nor 1e40.
skip("different float range", 42) if $vax_float;
skip("no inf",                42) unless $has_inf;

print "# Infinity\n";

my $BigDouble = eval '1e40';

# E.g. netbsd-alpha core dumps on Inf arith without this.
local $SIG{FPE} = sub { };

ok(Inf() > $BigDouble);  # This passes in netbsd-alpha.
ok(Inf() + $BigDouble > $BigDouble); # This coredumps in netbsd-alpha.
ok(Inf() + $BigDouble == Inf());
ok(Inf() - $BigDouble > $BigDouble);
ok(Inf() - $BigDouble == Inf());
ok(Inf() * $BigDouble > $BigDouble);
ok(Inf() * $BigDouble == Inf());
ok(Inf() / $BigDouble > $BigDouble);
ok(Inf() / $BigDouble == Inf());

ok(-Inf() < -$BigDouble);
ok(-Inf() + $BigDouble < $BigDouble);
ok(-Inf() + $BigDouble == -Inf());
ok(-Inf() - $BigDouble < -$BigDouble);
ok(-Inf() - $BigDouble == -Inf());
ok(-Inf() * $BigDouble < -$BigDouble);
ok(-Inf() * $BigDouble == -Inf());
ok(-Inf() / $BigDouble < -$BigDouble);
ok(-Inf() / $BigDouble == -Inf());

print "# sinh/sech/cosh/csch/tanh/coth unto infinity\n";

ok(near(sinh(100), eval '1.3441e+43', 1e-3));
ok(near(sech(100), eval '7.4402e-44', 1e-3));
ok(near(cosh(100), eval '1.3441e+43', 1e-3));
ok(near(csch(100), eval '7.4402e-44', 1e-3));
ok(near(tanh(100), 1));
ok(near(coth(100), 1));

ok(near(sinh(-100), eval '-1.3441e+43', 1e-3));
ok(near(sech(-100), eval ' 7.4402e-44', 1e-3));
ok(near(cosh(-100), eval ' 1.3441e+43', 1e-3));
ok(near(csch(-100), eval '-7.4402e-44', 1e-3));
ok(near(tanh(-100), -1));
ok(near(coth(-100), -1));

cmp_ok(sinh(1e5), '==', Inf());
cmp_ok(sech(1e5), '==', 0);
cmp_ok(cosh(1e5), '==', Inf());
cmp_ok(csch(1e5), '==', 0);
cmp_ok(tanh(1e5), '==', 1);
cmp_ok(coth(1e5), '==', 1);

cmp_ok(sinh(-1e5), '==', -Inf());
cmp_ok(sech(-1e5), '==', 0);
cmp_ok(cosh(-1e5), '==', Inf());
cmp_ok(csch(-1e5), '==', 0);
cmp_ok(tanh(-1e5), '==', -1);
cmp_ok(coth(-1e5), '==', -1);

}

print "# great_circle_distance with small angles\n";

for my $e (qw(1e-2 1e-3 1e-4 1e-5)) {
    # Can't assume == 0 because of floating point fuzz,
    # but let's hope for at least < $e.
    cmp_ok(great_circle_distance(0, $e, 0, $e), '<', $e,
           "great_circle_distance(0, $e, 0, $e) < $e");
}

for my $e (qw(1e-5 1e-6 1e-7 1e-8)) {
    # Verify that the distance is positive for points close together. A poor
    # algorithm is likely to give a distance of zero in some of these cases.
    cmp_ok(great_circle_distance(2, 2, 2, 2+$e), '>', 0,
           "great_circle_distance(2, 2, 2, " . (2+$e) . ") > 0");
}

print "# asin_real, acos_real\n";

is(acos_real(-2.0), pi);
is(acos_real(-1.0), pi);
is(acos_real(-0.5), acos(-0.5));
is(acos_real( 0.0), acos( 0.0));
is(acos_real( 0.5), acos( 0.5));
is(acos_real( 1.0), 0);
is(acos_real( 2.0), 0);

is(asin_real(-2.0), -&pip2);
is(asin_real(-1.0), -&pip2);
is(asin_real(-0.5), asin(-0.5));
is(asin_real( 0.0), asin( 0.0));
is(asin_real( 0.5), asin( 0.5));
is(asin_real( 1.0),  pip2);
is(asin_real( 2.0),  pip2);

# eof
