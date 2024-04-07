#!./perl 

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
}

plan tests => 4;

BEGIN {
    unshift @INC, sub {
	return () unless $_[1] =~ m#\At/(Foo|Bar)\.pm\z#;
	my $t = 0;
	return sub {
	    if(!$t) {
		$_ = "int(1,2);\n";
		$t = 1;
		$@ = "wibble";
		return 1;
	    } else {
		return 0;
	    }
	};
    };
}

is +(do "t/Bar.pm"), undef;
like $@, qr/\AToo many arguments for int /;
is eval { require "t/Foo.pm" }, undef;
like $@, qr/\AToo many arguments for int /;

1;
