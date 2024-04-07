#!./perl

BEGIN {
    chdir 't' if -d 't';
    require "./test.pl";
    set_up_inc(qw(../lib));
}

plan( tests => 5 );

my $long  = 'x' x 100;
my $short = 'abcd';

my @tests = (
    [ $long, 'long package name: one word' ],
    [ join( '::', $long, $long ), 'long package name: multiple words' ],
    [ join( q['], $long, $long ), q[long package name: multiple words using "'" separator] ],
    [ join( '::', $long, $short, $long ), 'long & short package name: multiple words' ],
    [ join( q['], $long, $short, $long ), q[long & short package name: multiple words using "'" separator] ],
);

foreach my $t (@tests) {
    my ( $sub, $name ) = @$t;

    fresh_perl_is(
        qq[no warnings qw(syntax deprecated); sub $sub { print qq[ok\n]} &{"$sub"}; my \$d = defined *{"foo$sub"} ],
        q[ok],
        { switches => ['-w'] },
        $name
    );
}
