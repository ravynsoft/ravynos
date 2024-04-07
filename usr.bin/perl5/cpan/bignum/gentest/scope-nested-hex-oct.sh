#!/bin/sh
# -*- mode: cperl; coding: utf-8-unix; -*-

eval 'exec ${PERL-perl} -Sx "$0" ${1+"$@"}'
  if 0;

#!perl
#line 9

use strict;
use warnings;

use File::Basename;

my $outfile = "t/scope-nested-hex-oct.t";

my $dirname = dirname(__FILE__);
chdir $dirname
  or die "$dirname: chdir failed: $!";

chomp(my $gitroot = `git rev-parse --show-toplevel`);
chdir $gitroot
  or die "$gitroot: chdir failed: $!";

open my($fh), ">", $outfile
  or die "$outfile: can't open file for writing: $!";

use Algorithm::Combinatorics 'permutations';

my $data = [
            ['bigint',   'Math::BigInt'  ],
            ['bigfloat', 'Math::BigFloat'],
            ['bigrat',   'Math::BigRat'  ],
           ];

print $fh <<'EOF' or die "$outfile: print failed: $!";
# -*- mode: perl; -*-

use strict;
use warnings;

use Test::More;

plan skip_all => 'Need at least Perl v5.10.1' if $] < "5.010001";

plan tests => 96;
EOF

my $iter = permutations([0, 1, 2]);
while (my $idxs = $iter -> next()) {

    my $p0 = $data -> [ $idxs -> [0] ][0];
    my $c0 = $data -> [ $idxs -> [0] ][1];
    my $p1 = $data -> [ $idxs -> [1] ][0];
    my $c1 = $data -> [ $idxs -> [1] ][1];
    my $p2 = $data -> [ $idxs -> [2] ][0];
    my $c2 = $data -> [ $idxs -> [2] ][1];

    print $fh <<"EOF" or die "$outfile: print failed: $!";

note "\\n$p0 -> $p1 -> $p2\\n\\n";

{
    note "use $p0;";
    use $p0;
    is(ref(hex("1")), "$c0", 'ref(hex("1"))');
    is(ref(oct("1")), "$c0", 'ref(oct("1"))');

    {
        note "use $p1;";
        use $p1;
        is(ref(hex("1")), "$c1", 'ref(hex("1"))');
        is(ref(oct("1")), "$c1", 'ref(oct("1"))');

        {
            note "use $p2;";
            use $p2;
            is(ref(hex("1")), "$c2", 'ref(hex("1"))');
            is(ref(oct("1")), "$c2", 'ref(oct("1"))');

            note "no $p2;";
            no $p2;
            is(ref(hex("1")), "", 'ref(hex("1"))');
            is(ref(oct("1")), "", 'ref(oct("1"))');
        }

        is(ref(hex("1")), "$c1", 'ref(hex("1"))');
        is(ref(oct("1")), "$c1", 'ref(oct("1"))');

        note "no $p1;";
        no $p1;
        is(ref(hex("1")), "", 'ref(hex("1"))');
        is(ref(oct("1")), "", 'ref(oct("1"))');
    }

    is(ref(hex("1")), "$c0", 'ref(hex("1"))');
    is(ref(oct("1")), "$c0", 'ref(oct("1"))');

    note "no $p0;";
    no $p0;
    is(ref(hex("1")), "", 'ref(hex("1"))');
    is(ref(oct("1")), "", 'ref(oct("1"))');
}
EOF
}

close($fh)
  or die "$outfile: can't close file after writing: $!";

print "Wrote '$outfile'\n";
