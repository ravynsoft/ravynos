#!./perl
#
# check if builtins behave as prototyped
#

# Ideally tests in t/comp wouldn't use require, as require isn't tested yet
print "1..16\n";

my $i = 1;

sub foo {}
my $bar = "bar";

sub test_too_many {
    eval $_[0];
    print "not " unless $@ =~ /^Too many arguments/;
    printf "ok %d\n",$i++;
}

sub test_too_few {
    eval $_[0];
    print "not " unless $@ =~ /^Not enough arguments/;
    printf "ok %d\n",$i++;
}

sub test_no_error {
    eval $_[0];
    print "not " if $@;
    printf "ok %d\n",$i++;
}

test_too_many($_) for split /\n/,
q[	defined(&foo, $bar);
	pos(1,$b);
	undef(&foo, $bar);
	uc($bar,$bar);
];

test_too_few($_) for split /\n/,
q[	unpack;
	pack;
];

test_no_error($_) for split /\n/,
q[	scalar(&foo,$bar);
	defined &foo, &foo, &foo;
	undef &foo, $bar;
	uc $bar,$bar;
	grep(not($bar), $bar);
	grep(not($bar, $bar), $bar);
	grep((not $bar, $bar, $bar), $bar);
        __FILE__();
        __LINE__();
        __PACKAGE__();
];
