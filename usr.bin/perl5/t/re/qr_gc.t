#!./perl -w

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
    undef &Regexp::DESTROY;
}

plan tests => 2;

my $destroyed;
{
    sub Regexp::DESTROY { $destroyed++ }
}

{
    my $rx = qr//;
}

is( $destroyed, 1, "destroyed regexp" );

undef $destroyed;

{
    my $var = bless {}, "Foo";
    my $rx = qr/(?{ $var })/;
}

is( $destroyed, 1, "destroyed regexp with closure capture" );

