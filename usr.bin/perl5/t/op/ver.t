#!./perl

BEGIN {
    chdir 't' if -d 't';
    $SIG{'__WARN__'} = sub { warn $_[0] if $DOWARN };
    require "./test.pl";
    set_up_inc( qw(. ../lib) );
    require "./charset_tools.pl";
}

$DOWARN = 1; # enable run-time warnings now

use Config;

plan( tests => 54 );

eval 'use v5.5.640';
is( $@, '', "use v5.5.640; $@");

require_ok('v5.5.640');

# printing characters should work
if ($::IS_ASCII) { # ASCII
    is('ok ',v111.107.32,'ASCII printing characters');

    # hash keys too
    $h{v111.107} = "ok";
    is('ok',$h{v111.107},'ASCII hash keys');
}
else { # EBCDIC
    is('ok ',v150.146.64,'EBCDIC printing characters');

    # hash keys too
    $h{v150.146} = "ok";
    is('ok',$h{v150.146},'EBCDIC hash keys');
}

# poetry optimization should also
sub v77 { "ok" }
$x = v77;
is('ok',$x,'poetry optimization');

# but not when dots are involved
if ($::IS_ASCII) { # ASCII
    $x = v77.78.79;
}
else {
    $x = v212.213.214;
}
is($x, 'MNO','poetry optimization with dots');

is(v1.20.300.4000, "\x{1}\x{14}\x{12c}\x{fa0}",'compare embedded \x{} string');

#
# now do the same without the "v"
eval 'use 5.5.640';
is( $@, '', "use 5.5.640; $@");

require_ok('5.5.640');

# hash keys too
if ($::IS_ASCII) { # ASCII
    $h{111.107.32} = "ok";
}
else {
    $h{150.146.64} = "ok";
}
is('ok',$h{ok },'hash keys w/o v');

if ($::IS_ASCII) { # ASCII
    $x = 77.78.79;
}
else {
    $x = 212.213.214;
}
is($x, 'MNO','poetry optimization with dots w/o v');

is(1.20.300.4000, "\x{1}\x{14}\x{12c}\x{fa0}",'compare embedded \x{} string w/o v');

# test sprintf("%vd"...) etc
if ($::IS_ASCII) { # ASCII
    is(sprintf("%vd", "Perl"), '80.101.114.108', 'ASCII sprintf("%vd", "Perl")');
}
else {
    is(sprintf("%vd", "Perl"), '215.133.153.147', 'EBCDIC sprintf("%vd", "Perl")');
}

is(sprintf("%vd", v1.22.333.4444), '1.22.333.4444', 'sprintf("%vd", v1.22.333.4444)');

if ($::IS_ASCII) { # ASCII
    is(sprintf("%vx", "Perl"), '50.65.72.6c', 'ASCII sprintf("%vx", "Perl")');
}
else {
    is(sprintf("%vx", "Perl"), 'd7.85.99.93', 'EBCDIC sprintf("%vx", "Perl")');
}

is(sprintf("%vX", 1.22.333.4444), '1.16.14D.115C','ASCII sprintf("%vX", 1.22.333.4444)');

if ($::IS_ASCII) { # ASCII
    is(sprintf("%#*vo", ":", "Perl"), '0120:0145:0162:0154', 'ASCII sprintf("%vo", "Perl")');
}
else {
    is(sprintf("%#*vo", ":", "Perl"), '0327:0205:0231:0223', 'EBCDIC sprintf("%vo", "Perl")');
}

is(sprintf("%*vb", "##", v1.22.333.4444),
    '1##10110##101001101##1000101011100', 'sprintf("%vb", 1.22.333.4444)');

is(sprintf("%vd", join("", map { chr }
			 unpack 'U*', pack('U*',2001,2002,2003))),
     '2001.2002.2003','unpack/pack U*');

{
    use bytes;

    if ($::IS_ASCII) { # ASCII
	is(sprintf("%vd", "Perl"), '80.101.114.108', 'ASCII sprintf("%vd", "Perl") w/use bytes');
    }
    else {
	is(sprintf("%vd", "Perl"), '215.133.153.147', 'EBCDIC sprintf("%vd", "Perl") w/use bytes');
    }

    if ($::IS_ASCII) { # ASCII
	is(sprintf("%vd", 1.22.333.4444), '1.22.197.141.225.133.156', 'ASCII sprintf("%vd", v1.22.333.4444 w/use bytes');
    }
    else {
	is(sprintf("%vd", 1.22.333.4444), '1.22.142.84.187.81.112', 'EBCDIC sprintf("%vd", v1.22.333.4444 w/use bytes');
    }

    if ($::IS_ASCII) { # ASCII
	is(sprintf("%vx", "Perl"), '50.65.72.6c', 'ASCII sprintf("%vx", "Perl")');
    }
    else {
	is(sprintf("%vx", "Perl"), 'd7.85.99.93', 'EBCDIC sprintf("%vx", "Perl")');
    }

    if ($::IS_ASCII) { # ASCII
	is(sprintf("%vX", v1.22.333.4444), '1.16.C5.8D.E1.85.9C', 'ASCII sprintf("%vX", v1.22.333.4444)');
    }
    else {
	is(sprintf("%vX", v1.22.333.4444), '1.16.8E.54.BB.51.70', 'EBCDIC sprintf("%vX", v1.22.333.4444)');
    }

    if ($::IS_ASCII) { # ASCII
	is(sprintf("%#*vo", ":", "Perl"), '0120:0145:0162:0154', 'ASCII sprintf("%#*vo", ":", "Perl")');
    }
    else {
	is(sprintf("%#*vo", ":", "Perl"), '0327:0205:0231:0223', 'EBCDIC sprintf("%#*vo", ":", "Perl")');
    }

    if ($::IS_ASCII) { # ASCII
	is(sprintf("%*vb", "##", v1.22.333.4444),
	     '1##10110##11000101##10001101##11100001##10000101##10011100',
	     'ASCII sprintf("%*vb", "##", v1.22.333.4444)');
    }
    else {
	is(sprintf("%*vb", "##", v1.22.333.4444),
            '1##10110##10001110##1010100##10111011##1010001##1110000',
	    'EBCDIC sprintf("%*vb", "##", v1.22.333.4444)');
    }
}

{
    # bug id 20000323.056 (#2641)

    is( "\x{41}",      +v65, 'bug id 20000323.056 (#2641)');
    is( "\x41",        +v65, 'bug id 20000323.056 (#2641)');
    is( "\x{c8}",     +v200, 'bug id 20000323.056 (#2641)');
    is( "\xc8",       +v200, 'bug id 20000323.056 (#2641)');
    is( "\x{221b}",  +v8731, 'bug id 20000323.056 (#2641)');
}

# See if the things Camel-III says are true: 29..33

# Chapter 2 pp67/68
my $vs = v1.20.300.4000;
is($vs,"\x{1}\x{14}\x{12c}\x{fa0}","v-string ne \\x{}");
is($vs,chr(1).chr(20).chr(300).chr(4000),"v-string ne chr()");
is('foo',($::IS_EBCDIC ? v134.150.150 : v102.111.111),"v-string ne ''");

# Chapter 15, pp403

# See if sane addr and gethostbyaddr() work
eval { require Socket; gethostbyaddr(v127.0.0.1, &Socket::AF_INET) };
if ($@) {
    # No - so do not test insane fails.
    $@ =~ s/\n/\n# /g;
}
SKIP: {
    skip("No Socket::AF_INET # $@") if $@;
    my $ip   = v2004.148.0.1;
    my $host;
    eval { $host = gethostbyaddr($ip,&Socket::AF_INET) };
    like($@, qr/Wide character/, "Non-bytes leak to gethostbyaddr");
}

# Chapter 28, pp671
ok(v5.6.0 lt v5.7.0, "v5.6.0 lt v5.7.0");

# part of 20000323.059 (#2644)
is(v200, chr(200),      "v200 eq chr(200)"      );
is(v200, +v200,         "v200 eq +v200"         );
is(v200, eval( "v200"), 'v200 eq "v200"'        );
is(v200, eval("+v200"), 'v200 eq eval("+v200")' );

# Tests for string/numeric value of $] itself
my ($revision,$version,$subversion) = split /\./, sprintf("%vd",$^V);

# $^V always displays the leading 'v' but we don't want that here
$revision =~ s/^v//;

print "# revision   = '$revision'\n";
print "# version    = '$version'\n";
print "# subversion = '$subversion'\n";

my $v = sprintf("%d.%.3d%.3d",$revision,$version,$subversion);

print "# v = '$v'\n";
print "# ] = '$]'\n";

is( $v, "$]", qq{\$^V eq "\$]"});

$v = $revision + $version/1000 + $subversion/1000000;

ok( abs($v - $]) < 10**-8 , "\$^V == \$] (numeric)" );

# Tests for magic v-strings 

$v = 1.2.3;
is( ref(\$v), 'VSTRING', 'v-string objects' );

$v = v1.2_3;
is( ref(\$v), 'VSTRING', 'v-string objects with v' );
is( sprintf("%vd", $v), '1.23', 'v-string ignores underscores' );

# [perl #16010]
%h = (v65 => 42);
ok( exists $h{v65}, "v-stringness is not engaged for vX" );
%h = (v65.66 => 42);
ok( exists $h{chr(65).chr(66)}, "v-stringness is engaged for vX.Y" );
%h = (65.66.67 => 42);
ok( exists $h{chr(65).chr(66).chr(67)}, "v-stringness is engaged for X.Y.Z" );

{
    local $|;
    $| = v0;
    $| = 1;
    --$|; --$|;
    is $|, 1, 'clobbering vstrings does not clobber all magic';
}

$a = $::IS_EBCDIC ? v134 : v102; $a =~ s/f/f/;
is ref \$a, 'SCALAR',
  's/// flattens vstrings even when the subst results in the same value';
$a = $::IS_EBCDIC ? v134 : v102; $a =~ y/f/g/;
is ref \$a, 'SCALAR', 'y/// flattens vstrings';

sub { $_[0] = v3;
      is ref \$h{nonexistent}, 'VSTRING', 'defelems can pass vstrings' }
->($h{nonexistent});

# The following tests whether v-strings are correctly
# interpreted by the tokeniser when it's in a XTERMORDORDOR
# state (fittingly, the only tokeniser state to contain the
# word MORDOR).

*{"\3"} = *DATA;
is( (readline v3), "This is what we expect to see!\n", "v-strings even work in Mordor" );

{
    # disable warnings just for the following test
    local $DOWARN = 0;

    # Keep a list of all warnings issued in this test
    my @warnings = ();
    local $SIG{__WARN__} = sub { push @warnings, @_; };

    # This should *not* result in a warning
    eval { my $foo; "This doesn't need to be here...".$foo; };
    is( scalar @warnings, 0, "Warnings are disabled by default pre 5.35" );

    # This *should* result in a warning
    eval { use v5.035; my $foo; "This doesn't need to be here...".$foo; };
    is( scalar @warnings, 1, "Warnings are enabled by default post 5.35" );
}

__DATA__
This is what we expect to see!
