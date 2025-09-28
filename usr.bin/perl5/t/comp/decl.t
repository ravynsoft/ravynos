#!./perl

# check to see if subroutine declarations work everywhere

sub one {
    print "ok 1\n";
}
format one =
ok 6
.

print "1..9\n";

one();
two();

sub two {
    print "ok 2\n";
}
format two =
@<<<
$foo
.

if ($x eq $x) {
    sub three {
	print "ok 3\n";
    }
    three();
}

four();
five();
$~ = 'one';
write;
$~ = 'two';
$foo = "ok 7";
write;
$~ = 'three';
write;

format three =
ok 8
.

if ($x eq $x) {
    goto quux;
}

print "not ok 9\n";
exit 1;

sub four {
    print "ok 4\n";
}

quux:
sub five {
    print "ok 5\n";
}
print "ok 9\n";
