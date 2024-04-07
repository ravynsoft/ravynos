#!./perl

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}

plan tests => 26;

use utf8;   # Tell EBCDIC translator to make this UTF-8,

eval {
    eval {
	die "Horribly\n";
    };
    die if $@;
};

like($@, qr/^Horribly/, 'die with no args propagates $@');
like($@, qr/\.{3}propagated at/, '... and appends a phrase');

{
    local $SIG{__DIE__} = sub { is( $_[0], "[\000]\n", 'Embedded null passed to signal handler' )};

    $err = "[\000]\n";
    eval {
        die $err;
    };
    is( $@, $err, 'Embedded null passed back into $@' );
}

{
    local $SIG{__DIE__} = sub {
	isa_ok( $_[0], 'ARRAY', 'pass an array ref as an argument' );
	$_[0]->[0]++;
    };
    $x = [3];
    eval { die $x; };

    is( $x->[0], 4, 'actual array, not a copy, passed to signal handler' );

    eval {
        eval {
            die [ 5 ];
        };
        die if $@;
    };

    is($@->[0], 7, 'die with no arguments propagates $@, but leaves references alone');

    eval {
	eval {
	    die bless [ 7 ], "Error";
	};
	isa_ok( $@, 'Error', '$@ is an Error object' );
	die if $@;
    };

    isa_ok( $@, 'Out', 'returning a different object than what was passed in, via PROPAGATE' );
    is($@->[0], 9, 'reference returned correctly');
}

{
    package Error;

    sub PROPAGATE {
	bless [$_[0]->[0]], "Out";
    }
}


{
    # die/warn and utf8
    use utf8;
    local $SIG{__DIE__};
    my $msg = "ce ºtii tu, bã ?\n";
    eval { die $msg };
    is( $@, $msg, "Literal passed to die" );
    our $err;
    local $SIG{__WARN__} = $SIG{__DIE__} = sub { $err = shift };
    eval { die $msg };
    is( $err, $msg, 'die handler with utf8' );
    eval { warn $msg };
    is( $err, $msg, 'warn handler with utf8' );
    eval qq/ use strict; \$\x{3b1} /;
    like( $@, qr/Global symbol "\$\x{3b1}"/, 'utf8 symbol names show up in $@' );
}

# [perl #36470] got uninit warning if $@ was undef

{
    use warnings "uninitialized";
    my $ok = 1;
    local $SIG{__DIE__};
    local $SIG{__WARN__} = sub { $ok = 0 };
    eval { undef $@; die };
    is( $ok, 1, 'no warnings if $@ is undef' );

    eval { $@ = 100; die };
    like($@."", qr/100\t\.{3}propagated at/,
         'check non-PVs in $@ are propagated');
}
{
    my @error;
    local $SIG{__DIE__}= sub { push @error, @_ };
    use strict;
    my $ok= eval '$intentionally_missing+1';
    my $eval_error= $@;
    is($ok,undef,"eval should return undef");
    is(0+@error,1,"we should have captured 1 error via __DIE__");
    like( $error[0],
          qr/Global symbol \"\$intentionally_missing\"/,
          "The __DIE__ handler should have seen this message");
    like( $eval_error,
          qr/Global symbol \"\$intentionally_missing\"/,
          "The eval error in '\$@' should contain this message");
    is( $error[0], $eval_error,
        "__DIE__ handler and \$@ should be the same");
}

TODO: {
    local $TODO = 'RT #4821: die qr{x} does not check termination';
    my $out = runperl(prog => 'die qr{x}', stderr => 1);
    like($out, qr/at -e line 1./, 'RT #4821: output from die qr{x}');
}
