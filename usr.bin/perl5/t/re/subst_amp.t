#!./perl -w

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}

use strict;

$_ = 'x' x 20; 
s/\d*|x/<$&>/g; 
my $foo = '<>' . ('<x><>' x 20) ;
is($_, $foo);

my $t = 'aaa';

$_ = $t;
my @res;
pos = 1;
s/\Ga(?{push @res, $_, $`})/xx/g;
is("$_ @res", 'axxxx aaa a aaa aa');

$_ = $t;
@res = ();
pos = 1;
s/\Ga(?{push @res, $_, $`})/x/g;
is("$_ @res", 'axx aaa a aaa aa');

$_ = $t;
@res = ();
pos = 1;
s/\Ga(?{push @res, $_, $`})/xx/;
is("$_ @res", 'axxa aaa a');

$_ = $t;
@res = ();
pos = 1;
s/\Ga(?{push @res, $_, $`})/x/;
is("$_ @res", 'axa aaa a');

$a = $t;
@res = ();
pos ($a) = 1;
$a =~ s/\Ga(?{push @res, $_, $`})/xx/g;
is("$a @res", 'axxxx aaa a aaa aa');

$a = $t;
@res = ();
pos ($a) = 1;
$a =~ s/\Ga(?{push @res, $_, $`})/x/g;
is("$a @res", 'axx aaa a aaa aa');

$a = $t;
@res = ();
pos ($a) = 1;
$a =~ s/\Ga(?{push @res, $_, $`})/xx/;
is("$a @res", 'axxa aaa a');

$a = $t;
@res = ();
pos ($a) = 1;
$a =~ s/\Ga(?{push @res, $_, $`})/x/;
is("$a @res", 'axa aaa a');

sub x2 {'xx'}
sub x1 {'x'}

$a = $t;
@res = ();
pos ($a) = 1;
$a =~ s/\Ga(?{push @res, $_, $`})/x2/ge;
is("$a @res", 'axxxx aaa a aaa aa');

$a = $t;
@res = ();
pos ($a) = 1;
$a =~ s/\Ga(?{push @res, $_, $`})/x1/ge;
is("$a @res", 'axx aaa a aaa aa');

$a = $t;
@res = ();
pos ($a) = 1;
$a =~ s/\Ga(?{push @res, $_, $`})/x2/e;
is("$a @res", 'axxa aaa a');

$a = $t;
@res = ();
pos ($a) = 1;
$a =~ s/\Ga(?{push @res, $_, $`})/x1/e;
is("$a @res", 'axa aaa a');

done_testing();
