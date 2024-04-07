#!./perl

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}

$| = 0; # test.pl now sets it on, which causes problems here.

use strict;	# Amazed that this hackery can be made strict ...
use Tie::Scalar;

# read in a file
sub cat {
    my $file = shift;
    local $/;
    open my $fh, $file or die "can't open '$file': $!";
    my $data = <$fh>;
    close $fh;
    $data;
}

# read in a utf-8 file
#
sub cat_utf8 {
    my $file = shift;
    local $/;
    open my $fh, '<', $file or die "can't open '$file': $!";
    binmode $fh, ':utf8';
    my $data = <$fh> // die "Can't read from '$file': $!";
    close $fh or die "error closing '$file': $!";
    $data;
}

# write a format to a plain file, then read it back in and compare

sub is_format {
    my ($glob, $want, $desc) = @_;
    local $::Level = $::Level + 1;
    my $file = 'Op_write.tmp';
    open $glob, '>', $file or die "Can't create '$file': $!";
    write $glob;
    close $glob or die "Could not close '$file': $!";
    is(cat($file), $want, $desc);
}

# write a format to a utf8 file, then read it back in and compare

sub is_format_utf8 {
    my ($glob, $want, $desc) = @_;
    local $::Level = $::Level + 1;
    my $file = 'Op_write.tmp';
    open $glob, '>:utf8', $file or die "Can't create '$file': $!";
    write $glob;
    close $glob or die "Could not close '$file': $!";
    is(cat_utf8($file), $want, $desc);
}

sub like_format_utf8 {
    my ($glob, $want, $desc) = @_;
    local $::Level = $::Level + 1;
    my $file = 'Op_write.tmp';
    open $glob, '>:utf8', $file or die "Can't create '$file': $!";
    write $glob;
    close $glob or die "Could not close '$file': $!";
    like(cat_utf8($file), $want, $desc);
}



#-- testing numeric fields in all variants (WL)

sub swrite {
    my $format = shift;
    local $^A = ""; # don't litter, use a local bin
    formline( $format, @_ );
    return $^A;
}

my @NumTests = (
    # [ format, value1, expected1, value2, expected2, .... ]
    [ '@###',           0,   '   0',         1, '   1',     9999.6, '####',
		9999.4999,   '9999',    -999.6, '####',     1e+100, '####' ],

    [ '@0##',           0,   '0000',         1, '0001',     9999.6, '####',
		-999.4999,   '-999',    -999.6, '####',     1e+100, '####' ],

    [ '^###',           0,   '   0',     undef, '    ' ],

    [ '^0##',           0,   '0000',     undef, '    ' ],

    [ '@###.',          0,  '   0.',         1, '   1.',    9999.6, '#####',
                9999.4999,  '9999.',    -999.6, '#####' ],

    [ '@##.##',         0, '  0.00',         1, '  1.00',  999.996, '######',
                999.99499, '999.99',      -100, '######' ],

    [ '@0#.##',         0, '000.00',         1, '001.00',       10, '010.00',
                  -0.0001, qr/^[\-0]00\.00$/ ],

);


my $num_tests = 0;
for my $tref ( @NumTests ){
    $num_tests += (@$tref - 1)/2;
}
#---------------------------------------------------------

# number of tests in section 1
my $bas_tests = 21;

# number of tests in section 3
my $bug_tests = 66 + 3 * 3 * 5 * 2 * 3 + 2 + 66 + 6 + 2 + 3 + 96 + 11 + 14
                + 12;

# number of tests in section 4
my $hmb_tests = 37;

my $tests = $bas_tests + $num_tests + $bug_tests + $hmb_tests;

plan $tests;

############
## Section 1
############

our ($fox, $multiline, $foo, $good);

format OUT =
the quick brown @<<
$fox
jumped
@*
$multiline
^<<<<<<<<<
$foo
^<<<<<<<<<
$foo
^<<<<<<...
$foo
now @<<the@>>>> for all@|||||men to come @<<<<
{
    'i' . 's', "time\n", $good, 'to'
}
.

open(OUT, '>Op_write.tmp') || die "Can't create Op_write.tmp";
END { unlink_all 'Op_write.tmp' }

$fox = 'foxiness';
$good = 'good';
$multiline = "forescore\nand\nseven years\n";
$foo = 'when in the course of human events it becomes necessary';
write(OUT);
close OUT or die "Could not close: $!";

my $right =
"the quick brown fox
jumped
forescore
and
seven years
when in
the course
of huma...
now is the time for all good men to come to\n";

is cat('Op_write.tmp'), $right and unlink_all 'Op_write.tmp';

$fox = 'wolfishness';
my $fox = 'foxiness';		# Test a lexical variable.

format OUT2 =
the quick brown @<<
$fox
jumped
@*
$multiline
^<<<<<<<<< ~~
$foo
now @<<the@>>>> for all@|||||men to come @<<<<
'i' . 's', "time\n", $good, 'to'
.

open OUT2, '>Op_write.tmp' or die "Can't create Op_write.tmp";

$good = 'good';
$multiline = "forescore\nand\nseven years\n";
$foo = 'when in the course of human events it becomes necessary';
write(OUT2);
close OUT2 or die "Could not close: $!";

$right =
"the quick brown fox
jumped
forescore
and
seven years
when in
the course
of human
events it
becomes
necessary
now is the time for all good men to come to\n";

is cat('Op_write.tmp'), $right and unlink_all 'Op_write.tmp';

eval <<'EOFORMAT';
format OUT2 =
the brown quick @<<
$fox
jumped
@*
$multiline
and
^<<<<<<<<< ~~
$foo
now @<<the@>>>> for all@|||||men to come @<<<<
'i' . 's', "time\n", $good, 'to'
.
EOFORMAT

open(OUT2, '>Op_write.tmp') || die "Can't create Op_write.tmp";

$fox = 'foxiness';
$good = 'good';
$multiline = "forescore\nand\nseven years\n";
$foo = 'when in the course of human events it becomes necessary';
write(OUT2);
close OUT2 or die "Could not close: $!";

$right =
"the brown quick fox
jumped
forescore
and
seven years
and
when in
the course
of human
events it
becomes
necessary
now is the time for all good men to come to\n";

is cat('Op_write.tmp'), $right and unlink_all 'Op_write.tmp';

# formline tests

$right = <<EOT;
@ a
@> ab
@>> abc
@>>>  abc
@>>>>   abc
@>>>>>    abc
@>>>>>>     abc
@>>>>>>>      abc
@>>>>>>>>       abc
@>>>>>>>>>        abc
@>>>>>>>>>>         abc
EOT

my $was1 = my $was2 = '';
our $format2;
for (0..10) {           
  # lexical picture
  $^A = '';
  my $format1 = '@' . '>' x $_;
  formline $format1, 'abc';
  $was1 .= "$format1 $^A\n";
  # global
  $^A = '';
  local $format2 = '@' . '>' x $_;
  formline $format2, 'abc';
  $was2 .= "$format2 $^A\n";
}
is $was1, $right;
is $was2, $right;

$^A = '';

# more test

format OUT3 =
^<<<<<<...
$foo
.

open(OUT3, '>Op_write.tmp') || die "Can't create Op_write.tmp";

$foo = 'fit          ';
write(OUT3);
close OUT3 or die "Could not close: $!";

$right =
"fit\n";

is cat('Op_write.tmp'), $right and unlink_all 'Op_write.tmp';


# test lexicals and globals
{
    my $test = curr_test();
    my $this = "ok";
    our $that = $test;
    format LEX =
@<<@|
$this,$that
.
    open(LEX, ">&STDOUT") or die;
    write LEX;
    $that = ++$test;
    write LEX;
    close LEX or die "Could not close: $!";
    curr_test($test + 1);
}
# LEX_INTERPNORMAL test
my %e = ( a => 1 );
format OUT4 =
@<<<<<<
"$e{a}"
.
open   OUT4, ">Op_write.tmp" or die "Can't create Op_write.tmp";
write (OUT4);
close  OUT4 or die "Could not close: $!";
is cat('Op_write.tmp'), "1\n" and unlink_all "Op_write.tmp";

# More LEX_INTERPNORMAL
format OUT4a=
@<<<<<<<<<<<<<<<
"${; use
     strict; \'Nasdaq dropping like flies'}"
.
open   OUT4a, ">Op_write.tmp" or die "Can't create Op_write.tmp";
write (OUT4a);
close  OUT4a or die "Could not close: $!";
is cat('Op_write.tmp'), "Nasdaq dropping\n", 'skipspace inside "${...}"'
    and unlink_all "Op_write.tmp";

our $test1;
eval <<'EOFORMAT';
format OUT10 =
@####.## @0###.##
$test1, $test1
.
EOFORMAT

open(OUT10, '>Op_write.tmp') || die "Can't create Op_write.tmp";

$test1 = 12.95;
write(OUT10);
close OUT10 or die "Could not close: $!";

$right = "   12.95 00012.95\n";
is cat('Op_write.tmp'), $right and unlink_all 'Op_write.tmp';

eval <<'EOFORMAT';
format OUT11 =
@0###.## 
$test1
@ 0#
$test1
@0 # 
$test1
.
EOFORMAT

open(OUT11, '>Op_write.tmp') || die "Can't create Op_write.tmp";

$test1 = 12.95;
write(OUT11);
close OUT11 or die "Could not close: $!";

$right = 
"00012.95
1 0#
10 #\n";
is cat('Op_write.tmp'), $right and unlink_all 'Op_write.tmp';

{
    my $test = curr_test();
    my $el;
    format OUT12 =
ok ^<<<<<<<<<<<<<<~~ # sv_chop() naze
$el
.
    my %hash = ($test => 3);
    open(OUT12, '>Op_write.tmp') || die "Can't create Op_write.tmp";

    for $el (keys %hash) {
	write(OUT12);
    }
    close OUT12 or die "Could not close: $!";
    print cat('Op_write.tmp');
    curr_test($test + 1);
}

{
    my $test = curr_test();
    # Bug report and testcase by Alexey Tourbin
    my $v;
    tie $v, 'Tie::StdScalar';
    $v = $test;
    format OUT13 =
ok ^<<<<<<<<< ~~
$v
.
    open(OUT13, '>Op_write.tmp') || die "Can't create Op_write.tmp";
    write(OUT13);
    close OUT13 or die "Could not close: $!";
    print cat('Op_write.tmp');
    curr_test($test + 1);
}

{   # test 14
    # Bug #24774 format without trailing \n failed assertion, but this
    # must fail since we have a trailing ; in the eval'ed string (WL)
    my @v = ('k');
    eval "format OUT14 = \n@\n\@v";
    like $@, qr/Format not terminated/;
}

{   # test 15
    # text lost in ^<<< field with \r in value (WL)
    my $txt = "line 1\rline 2";
    format OUT15 =
^<<<<<<<<<<<<<<<<<<
$txt
^<<<<<<<<<<<<<<<<<<
$txt
.
    open(OUT15, '>Op_write.tmp') || die "Can't create Op_write.tmp";
    write(OUT15);
    close OUT15 or die "Could not close: $!";
    my $res = cat('Op_write.tmp');
    is $res, "line 1\nline 2\n";
}

{   # test 16: multiple use of a variable in same line with ^<
    my $txt = "this_is_block_1 this_is_block_2 this_is_block_3 this_is_block_4";
    format OUT16 =
^<<<<<<<<<<<<<<<< ^<<<<<<<<<<<<<<<<
$txt,             $txt
^<<<<<<<<<<<<<<<< ^<<<<<<<<<<<<<<<<
$txt,             $txt
.
    open(OUT16, '>Op_write.tmp') || die "Can't create Op_write.tmp";
    write(OUT16);
    close OUT16 or die "Could not close: $!";
    my $res = cat('Op_write.tmp');
    is $res, <<EOD;
this_is_block_1   this_is_block_2
this_is_block_3   this_is_block_4
EOD
}

{   # test 17: @* "should be on a line of its own", but it should work
    # cleanly with literals before and after. (WL)

    my $txt = "This is line 1.\nThis is the second line.\nThird and last.\n";
    format OUT17 =
Here we go: @* That's all, folks!
            $txt
.
    open(OUT17, '>Op_write.tmp') || die "Can't create Op_write.tmp";
    write(OUT17);
    close OUT17 or die "Could not close: $!";
    my $res = cat('Op_write.tmp');
    chomp( $txt );
    my $exp = <<EOD;
Here we go: $txt That's all, folks!
EOD
    is $res, $exp;
}

{   # test 18: @# and ~~ would cause runaway format, but we now
    # catch this while compiling (WL)

    format OUT18 =
@######## ~~
10
.
    open(OUT18, '>Op_write.tmp') || die "Can't create Op_write.tmp";
    eval { write(OUT18); };
    like $@,  qr/Repeated format line will never terminate/;
    close OUT18 or die "Could not close: $!";
}

{   # test 19: \0 in an evel'ed format, doesn't cause empty lines (WL)
    my $v = 'gaga';
    eval "format OUT19 = \n" .
         '@<<<' . "\0\n" .
         '$v' .   "\n" .
         '@<<<' . "\0\n" .
         '$v' . "\n.\n";
    open(OUT19, '>Op_write.tmp') || die "Can't create Op_write.tmp";
    write(OUT19);
    close OUT19 or die "Could not close: $!";
    my $res = cat('Op_write.tmp');
    is $res, <<EOD;
gaga\0
gaga\0
EOD
}

{   # test 20: hash accesses; single '}' must not terminate format '}' (WL)
    my %h = ( xkey => 'xval', ykey => 'yval' );
    format OUT20 =
@>>>> @<<<< ~~
each %h
@>>>> @<<<<
$h{xkey}, $h{ykey}
@>>>> @<<<<
{ $h{xkey}, $h{ykey}
}
}
.
    my $exp = '';
    while( my( $k, $v ) = each( %h ) ){
	$exp .= sprintf( "%5s %s\n", $k, $v );
    }
    $exp .= sprintf( "%5s %s\n", $h{xkey}, $h{ykey} );
    $exp .= sprintf( "%5s %s\n", $h{xkey}, $h{ykey} );
    $exp .= "}\n";
    open(OUT20, '>Op_write.tmp') || die "Can't create Op_write.tmp";
    write(OUT20);
    close OUT20 or die "Could not close: $!";
    my $res = cat('Op_write.tmp');
    is $res, $exp;
}


#####################
## Section 2
## numeric formatting
#####################

curr_test($bas_tests + 1);

for my $tref ( @NumTests ){
    my $writefmt = shift( @$tref );
    while (@$tref) {
	my $val      = shift @$tref;
	my $expected = shift @$tref;
        my $writeres = swrite( $writefmt, $val );
	if (ref $expected) {
	    like $writeres, $expected, $writefmt;
	} else {
	    is $writeres, $expected, $writefmt;
	}	
    }
}


#####################################
## Section 3
## Easiest to add new tests just here
#####################################

# DAPM. Exercise a couple of error codepaths

{
    local $~ = '';
    eval { write };
    like $@, qr/Undefined format ""/, 'format with 0-length name';

    $~ = "\0foo";
    eval { write };
    like $@, qr/Undefined format "\0foo"/,
	'no such format beginning with null';

    $~ = "NOSUCHFORMAT";
    eval { write };
    like $@, qr/Undefined format "NOSUCHFORMAT"/, 'no such format';
}

select +(select(OUT21), do {
    open(OUT21, '>Op_write.tmp') || die "Can't create Op_write.tmp";

    format OUT21 =
@<<
$_
.

    local $^ = '';
    local $= = 1;
    $_ = "aataaaaaaaaaaaaaa"; eval { write(OUT21) };
    like $@, qr/Undefined top format ""/, 'top format with 0-length name';

    $^ = "\0foo";
    # For some reason, we have to do this twice to get the error again.
    $_ = "aataaaaaaaaaaaaaa"; eval { write(OUT21) };
    $_ = "aataaaaaaaaaaaaaa"; eval { write(OUT21) };
    like $@, qr/Undefined top format "\0foo"/,
	'no such top format beginning with null';

    $^ = "NOSUCHFORMAT";
    $_ = "aataaaaaaaaaaaaaa"; eval { write(OUT21) };
    $_ = "aataaaaaaaaaaaaaa"; eval { write(OUT21) };
    like $@, qr/Undefined top format "NOSUCHFORMAT"/, 'no such top format';

    # reset things;
    eval { write(OUT21) };
    undef $^A;

    close OUT21 or die "Could not close: $!";
})[0];



# [perl #119847],  [perl #119849], [perl #119851]
# Non-real vars like tied, overloaded and refs could, when stringified,
# fail to be processed properly, causing infinite loops on ~~, utf8
# warnings etc, ad nauseum.


my $u22a = "N" x 8;

format OUT22a =
'^<<<<<<<<'~~
$u22a
.

is_format_utf8(\*OUT22a,
               "'NNNNNNNN '\n");


my $u22b = "N" x 8;
utf8::upgrade($u22b);

format OUT22b =
'^<<<<<<<<'~~
$u22b
.

is_format_utf8(\*OUT22b,
               "'NNNNNNNN '\n");

my $u22c = "\x{FF}" x 8;

format OUT22c =
'^<<<<<<<<'~~
$u22c
.

is_format_utf8(\*OUT22c,
               "'\x{FF}\x{FF}\x{FF}\x{FF}\x{FF}\x{FF}\x{FF}\x{FF} '\n");

my $u22d = "\x{FF}" x 8;
utf8::upgrade($u22d);

format OUT22d =
'^<<<<<<<<'~~
$u22d
.

is_format_utf8(\*OUT22d,
               "'\x{FF}\x{FF}\x{FF}\x{FF}\x{FF}\x{FF}\x{FF}\x{FF} '\n");

my $u22e = "\x{100}" x 8;

format OUT22e =
'^<<<<<<<<'~~
$u22e
.

is_format_utf8(\*OUT22e,
               "'\x{100}\x{100}\x{100}\x{100}\x{100}\x{100}\x{100}\x{100} '\n");


my $u22f = "N" x 8;

format OUT22f =
'^<'~~
$u22f
.

is_format_utf8(\*OUT22f,
               "'NN'\n"x4);


my $u22g = "N" x 8;
utf8::upgrade($u22g);

format OUT22g =
'^<'~~
$u22g
.

is_format_utf8(\*OUT22g,
               "'NN'\n"x4);

my $u22h = "\x{FF}" x 8;

format OUT22h =
'^<'~~
$u22h
.

is_format_utf8(\*OUT22h,
               "'\x{FF}\x{FF}'\n"x4);

my $u22i = "\x{FF}" x 8;
utf8::upgrade($u22i);

format OUT22i =
'^<'~~
$u22i
.

is_format_utf8(\*OUT22i,
               "'\x{FF}\x{FF}'\n"x4);

my $u22j = "\x{100}" x 8;

format OUT22j =
'^<'~~
$u22j
.

is_format_utf8(\*OUT22j,
               "'\x{100}\x{100}'\n"x4);


tie my $u23a, 'Tie::StdScalar';
$u23a = "N" x 8;

format OUT23a =
'^<<<<<<<<'~~
$u23a
.

is_format_utf8(\*OUT23a,
               "'NNNNNNNN '\n");


tie my $u23b, 'Tie::StdScalar';
$u23b = "N" x 8;
utf8::upgrade($u23b);

format OUT23b =
'^<<<<<<<<'~~
$u23b
.

is_format_utf8(\*OUT23b,
               "'NNNNNNNN '\n");

tie my $u23c, 'Tie::StdScalar';
$u23c = "\x{FF}" x 8;

format OUT23c =
'^<<<<<<<<'~~
$u23c
.

is_format_utf8(\*OUT23c,
               "'\x{FF}\x{FF}\x{FF}\x{FF}\x{FF}\x{FF}\x{FF}\x{FF} '\n");

tie my $u23d, 'Tie::StdScalar';
my $temp = "\x{FF}" x 8;
utf8::upgrade($temp);
$u23d = $temp;

format OUT23d =
'^<<<<<<<<'~~
$u23d
.

is_format_utf8(\*OUT23d,
               "'\x{FF}\x{FF}\x{FF}\x{FF}\x{FF}\x{FF}\x{FF}\x{FF} '\n");

tie my $u23e, 'Tie::StdScalar';
$u23e = "\x{100}" x 8;

format OUT23e =
'^<<<<<<<<'~~
$u23e
.

is_format_utf8(\*OUT23e,
               "'\x{100}\x{100}\x{100}\x{100}\x{100}\x{100}\x{100}\x{100} '\n");

tie my $u23f, 'Tie::StdScalar';
$u23f = "N" x 8;

format OUT23f =
'^<'~~
$u23f
.

is_format_utf8(\*OUT23f,
               "'NN'\n"x4);


tie my $u23g, 'Tie::StdScalar';
my $temp = "N" x 8;
utf8::upgrade($temp);
$u23g = $temp;

format OUT23g =
'^<'~~
$u23g
.

is_format_utf8(\*OUT23g,
               "'NN'\n"x4);

tie my $u23h, 'Tie::StdScalar';
$u23h = "\x{FF}" x 8;

format OUT23h =
'^<'~~
$u23h
.

is_format_utf8(\*OUT23h,
               "'\x{FF}\x{FF}'\n"x4);

$temp = "\x{FF}" x 8;
utf8::upgrade($temp);
tie my $u23i, 'Tie::StdScalar';
$u23i = $temp;

format OUT23i =
'^<'~~
$u23i
.

is_format_utf8(\*OUT23i,
               "'\x{FF}\x{FF}'\n"x4);

tie my $u23j, 'Tie::StdScalar';
$u23j = "\x{100}" x 8;

format OUT23j =
'^<'~~
$u23j
.

is_format_utf8(\*OUT23j,
               "'\x{100}\x{100}'\n"x4);

{
    package UTF8Toggle;

    sub TIESCALAR {
        my $class = shift;
        my $value = shift;
        my $state = shift||0;
        return bless [$value, $state], $class;
    }

    sub FETCH {
        my $self = shift;
        $self->[1] = ! $self->[1];
        if ($self->[1]) {
           utf8::downgrade($self->[0]);
        } else {
           utf8::upgrade($self->[0]);
        }
        $self->[0];
    }

   sub STORE {
       my $self = shift;
       $self->[0] = shift;
    }
}

tie my $u24a, 'UTF8Toggle';
$u24a = "N" x 8;

format OUT24a =
'^<<<<<<<<'~~
$u24a
.

is_format_utf8(\*OUT24a,
               "'NNNNNNNN '\n");


tie my $u24b, 'UTF8Toggle';
$u24b = "N" x 8;
utf8::upgrade($u24b);

format OUT24b =
'^<<<<<<<<'~~
$u24b
.

is_format_utf8(\*OUT24b,
               "'NNNNNNNN '\n");

tie my $u24c, 'UTF8Toggle';
$u24c = "\x{FF}" x 8;

format OUT24c =
'^<<<<<<<<'~~
$u24c
.

is_format_utf8(\*OUT24c,
               "'\x{FF}\x{FF}\x{FF}\x{FF}\x{FF}\x{FF}\x{FF}\x{FF} '\n");

tie my $u24d, 'UTF8Toggle', 1;
$u24d = "\x{FF}" x 8;

format OUT24d =
'^<<<<<<<<'~~
$u24d
.

is_format_utf8(\*OUT24d,
               "'\x{FF}\x{FF}\x{FF}\x{FF}\x{FF}\x{FF}\x{FF}\x{FF} '\n");



tie my $u24f, 'UTF8Toggle';
$u24f = "N" x 8;

format OUT24f =
'^<'~~
$u24f
.

is_format_utf8(\*OUT24f,
               "'NN'\n"x4);


tie my $u24g, 'UTF8Toggle';
my $temp = "N" x 8;
utf8::upgrade($temp);
$u24g = $temp;

format OUT24g =
'^<'~~
$u24g
.

is_format_utf8(\*OUT24g,
               "'NN'\n"x4);

tie my $u24h, 'UTF8Toggle';
$u24h = "\x{FF}" x 8;

format OUT24h =
'^<'~~
$u24h
.

is_format_utf8(\*OUT24h,
               "'\x{FF}\x{FF}'\n"x4);

tie my $u24i, 'UTF8Toggle', 1;
$u24i = "\x{FF}" x 8;

format OUT24i =
'^<'~~
$u24i
.

is_format_utf8(\*OUT24i,
               "'\x{FF}\x{FF}'\n"x4);

{
    package OS;
    use overload '""' => sub { ${$_[0]}; };

    sub new {
        my ($class, $value) = @_;
        bless \$value, $class;
    }
}

my $u25a = OS->new("N" x 8);

format OUT25a =
'^<<<<<<<<'~~
$u25a
.

is_format_utf8(\*OUT25a,
               "'NNNNNNNN '\n");


my $temp = "N" x 8;
utf8::upgrade($temp);
my $u25b = OS->new($temp);

format OUT25b =
'^<<<<<<<<'~~
$u25b
.

is_format_utf8(\*OUT25b,
               "'NNNNNNNN '\n");

my $u25c = OS->new("\x{FF}" x 8);

format OUT25c =
'^<<<<<<<<'~~
$u25c
.

is_format_utf8(\*OUT25c,
               "'\x{FF}\x{FF}\x{FF}\x{FF}\x{FF}\x{FF}\x{FF}\x{FF} '\n");

$temp = "\x{FF}" x 8;
utf8::upgrade($temp);
my $u25d = OS->new($temp);

format OUT25d =
'^<<<<<<<<'~~
$u25d
.

is_format_utf8(\*OUT25d,
               "'\x{FF}\x{FF}\x{FF}\x{FF}\x{FF}\x{FF}\x{FF}\x{FF} '\n");

my $u25e = OS->new("\x{100}" x 8);

format OUT25e =
'^<<<<<<<<'~~
$u25e
.

is_format_utf8(\*OUT25e,
               "'\x{100}\x{100}\x{100}\x{100}\x{100}\x{100}\x{100}\x{100} '\n");


my $u25f = OS->new("N" x 8);

format OUT25f =
'^<'~~
$u25f
.

is_format_utf8(\*OUT25f,
               "'NN'\n"x4);


$temp = "N" x 8;
utf8::upgrade($temp);
my $u25g = OS->new($temp);

format OUT25g =
'^<'~~
$u25g
.

is_format_utf8(\*OUT25g,
               "'NN'\n"x4);

my $u25h = OS->new("\x{FF}" x 8);

format OUT25h =
'^<'~~
$u25h
.

is_format_utf8(\*OUT25h,
               "'\x{FF}\x{FF}'\n"x4);

$temp = "\x{FF}" x 8;
utf8::upgrade($temp);
my $u25i = OS->new($temp);

format OUT25i =
'^<'~~
$u25i
.

is_format_utf8(\*OUT25i,
               "'\x{FF}\x{FF}'\n"x4);

my $u25j = OS->new("\x{100}" x 8);

format OUT25j =
'^<'~~
$u25j
.

is_format_utf8(\*OUT25j,
               "'\x{100}\x{100}'\n"x4);

{
    package OS::UTF8Toggle;
    use overload '""' => sub {
        my $self = shift;
        $self->[1] = ! $self->[1];
        if ($self->[1]) {
            utf8::downgrade($self->[0]);
        } else {
            utf8::upgrade($self->[0]);
        }
        $self->[0];
    };

    sub new {
        my ($class, $value, $state) = @_;
        bless [$value, $state], $class;
    }
}


my $u26a = OS::UTF8Toggle->new("N" x 8);

format OUT26a =
'^<<<<<<<<'~~
$u26a
.

is_format_utf8(\*OUT26a,
               "'NNNNNNNN '\n");


my $u26b = OS::UTF8Toggle->new("N" x 8, 1);

format OUT26b =
'^<<<<<<<<'~~
$u26b
.

is_format_utf8(\*OUT26b,
               "'NNNNNNNN '\n");

my $u26c = OS::UTF8Toggle->new("\x{FF}" x 8);

format OUT26c =
'^<<<<<<<<'~~
$u26c
.

is_format_utf8(\*OUT26c,
               "'\x{FF}\x{FF}\x{FF}\x{FF}\x{FF}\x{FF}\x{FF}\x{FF} '\n");

my $u26d = OS::UTF8Toggle->new("\x{FF}" x 8, 1);

format OUT26d =
'^<<<<<<<<'~~
$u26d
.

is_format_utf8(\*OUT26d,
               "'\x{FF}\x{FF}\x{FF}\x{FF}\x{FF}\x{FF}\x{FF}\x{FF} '\n");


my $u26f = OS::UTF8Toggle->new("N" x 8);

format OUT26f =
'^<'~~
$u26f
.

is_format_utf8(\*OUT26f,
               "'NN'\n"x4);


my $u26g = OS::UTF8Toggle->new("N" x 8, 1);

format OUT26g =
'^<'~~
$u26g
.

is_format_utf8(\*OUT26g,
               "'NN'\n"x4);

my $u26h = OS::UTF8Toggle->new("\x{FF}" x 8);

format OUT26h =
'^<'~~
$u26h
.

is_format_utf8(\*OUT26h,
               "'\x{FF}\x{FF}'\n"x4);

my $u26i = OS::UTF8Toggle->new("\x{FF}" x 8, 1);

format OUT26i =
'^<'~~
$u26i
.

is_format_utf8(\*OUT26i,
               "'\x{FF}\x{FF}'\n"x4);



{
    my $zero = $$ - $$;

    package Number;

    sub TIESCALAR {
        my $class = shift;
        my $value = shift;
        return bless \$value, $class;
    }

    # The return value should always be SvNOK() only:
    sub FETCH {
        my $self = shift;
        # avoid "" getting converted to "0" and thus
        # causing an infinite loop
        return "" unless length ($$self);
        return $$self - 0.5 + $zero + 0.5;
    }

   sub STORE {
       my $self = shift;
       $$self = shift;
    }

   package ONumber;

   use overload '""' => sub {
        my $self = shift;
        return $$self - 0.5 + $zero + 0.5;
    };

    sub new {
       my $class = shift;
       my $value = shift;
       return bless \$value, $class;
   }
}

my $v27a = 1/256;

format OUT27a =
'^<<<<<<<<<'~~
$v27a
.

is_format_utf8(\*OUT27a,
               "'0.00390625'\n");

my $v27b = 1/256;

format OUT27b =
'^<'~~
$v27b
.

is_format_utf8(\*OUT27b,
               "'0.'\n'00'\n'39'\n'06'\n'25'\n");

tie my $v27c, 'Number', 1/256;

format OUT27c =
'^<<<<<<<<<'~~
$v27c
.

is_format_utf8(\*OUT27c,
               "'0.00390625'\n");

my $v27d = 1/256;

format OUT27d =
'^<'~~
$v27d
.

is_format_utf8(\*OUT27d,
               "'0.'\n'00'\n'39'\n'06'\n'25'\n");

my $v27e = ONumber->new(1/256);

format OUT27e =
'^<<<<<<<<<'~~
$v27e
.

is_format_utf8(\*OUT27e,
               "'0.00390625'\n");

my $v27f = ONumber->new(1/256);

format OUT27f =
'^<'~~
$v27f
.

is_format_utf8(\*OUT27f,
               "'0.'\n'00'\n'39'\n'06'\n'25'\n");

{
    package Ref;
    use overload '""' => sub {
	return ${$_[0]};
    };

    sub new {
       my $class = shift;
       my $value = shift;
       return bless \$value, $class;
   }
}

my $v28a = {};

format OUT28a =
'^<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<'~~
$v28a
.


# 'HASH(0x1716b60)     '
my $qr_hash   = qr/^'HASH\(0x[0-9a-f]+\)\s+'\n$/;

# 'HASH'
# '(0x1'
# '716b'
# 'c0) '
my $qr_hash_m = qr/^'HASH'\n('[0-9a-fx() ]{4}'\n)+$/;

like_format_utf8(\*OUT28a, $qr_hash);

my $v28b = {};

format OUT28b =
'^<<<'~~
$v28b
.

like_format_utf8(\*OUT28b, $qr_hash_m);


tie my $v28c, 'Tie::StdScalar';
$v28c = {};

format OUT28c =
'^<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<'~~
$v28c
.

like_format_utf8(\*OUT28c, $qr_hash);

tie my $v28d, 'Tie::StdScalar';
$v28d = {};

format OUT28d =
'^<<<'~~
$v28d
.

like_format_utf8(\*OUT28d, $qr_hash_m);

my $v28e = Ref->new({});

format OUT28e =
'^<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<'~~
$v28e
.

like_format_utf8(\*OUT28e, $qr_hash);

my $v28f = Ref->new({});

format OUT28f =
'^<<<'~~
$v28f
.

like_format_utf8(\*OUT28f, $qr_hash_m);

my $v29a;
format OUT29a =
[^<<<]~~
$v29a
.

$v29a = "-ABCD";
is_format(\*OUT29a, "[-   ]\n[ABCD]\n");

$v29a = "A-BCD";
is_format(\*OUT29a, "[A-  ]\n[BCD ]\n");

$v29a = "AB-CD";
is_format(\*OUT29a, "[AB- ]\n[CD  ]\n");

$v29a = "ABC-D";
is_format(\*OUT29a, "[ABC-]\n[D   ]\n");

$v29a = "ABCD-";
is_format(\*OUT29a, "[ABCD]\n[-   ]\n");

$v29a = "ABCDE-";
is_format(\*OUT29a, "[ABCD]\n[E-  ]\n");

$v29a = "-ABCD";
is_format_utf8(\*OUT29a, "[-   ]\n[ABCD]\n");

$v29a = "A-BCD";
is_format_utf8(\*OUT29a, "[A-  ]\n[BCD ]\n");

$v29a = "AB-CD";
is_format_utf8(\*OUT29a, "[AB- ]\n[CD  ]\n");

$v29a = "ABC-D";
is_format_utf8(\*OUT29a, "[ABC-]\n[D   ]\n");

$v29a = "ABCD-";
is_format_utf8(\*OUT29a, "[ABCD]\n[-   ]\n");

$v29a = "ABCDE-";
is_format_utf8(\*OUT29a, "[ABCD]\n[E-  ]\n");


{
  package Count;

  sub TIESCALAR {
    my $class = shift;
    bless [shift, 0, 0], $class;
  }

  sub FETCH {
    my $self = shift;
    ++$self->[1];
    $self->[0];
  }

  sub STORE {
    my $self = shift;
    ++$self->[2];
    $self->[0] = shift;
  }
}

{
  my ($pound_utf8, $pm_utf8) = map { my $a = "$_\x{100}"; chop $a; $a}
    my ($pound, $pm) = ("\xA3", "\xB1");

  foreach my $first ('N', $pound, $pound_utf8) {
    foreach my $base ('N', $pm, $pm_utf8) {
      foreach my $second ($base, "$base\n", "$base\nMoo!", "$base\nMoo!\n",
			  "$base\nMoo!\n",) {
	foreach (['^*', qr/(.+)/], ['@*', qr/(.*?)$/s]) {
	  my ($format, $re) = @$_;
	  $format = "1^*2 3${format}4";
	  foreach my $class ('', 'Count') {
	    my $name = qq{swrite("$format", "$first", "$second") class="$class"};
	    $name =~ s/\n/\\n/g;
	    $name =~ s{(.)}{
			ord($1) > 126 ? sprintf("\\x{%x}",ord($1)) : $1
		    }ge;

	    $first =~ /(.+)/ or die $first;
	    my $expect = "1${1}2";
	    $second =~ $re or die $second;
	    $expect .= " 3${1}4";

	    if ($class) {
	      my $copy1 = $first;
	      my $copy2;
	      tie $copy2, $class, $second;
	      is swrite("$format", $copy1, $copy2), $expect, $name;
	      my $obj = tied $copy2;
	      is $obj->[1], 1, 'value read exactly once';
	    } else {
	      my ($copy1, $copy2) = ($first, $second);
	      is swrite("$format", $copy1, $copy2), $expect, $name;
	    }
	  }
	}
      }
    }
  }
}

{
  # This will fail an assertion in 5.10.0 built with -DDEBUGGING (because
  # pp_formline attempts to set SvCUR() on an SVt_RV). I suspect that it will
  # be doing something similarly out of bounds on everything from 5.000
  my $ref = [];
  my $exp = ">$ref<";
  is swrite('>^*<', $ref), $exp;
  $ref = [];
  my $exp = ">$ref<";
  is swrite('>@*<', $ref), $exp;
}

format EMPTY =
.

my $test = curr_test();

format Comment =
ok @<<<<<
$test
.


# RT #8698 format bug with undefined _TOP

open STDOUT_DUP, ">&STDOUT";
my $oldfh = select STDOUT_DUP;
$= = 10;
{
  local $~ = "Comment";
  write;
  curr_test($test + 1);
  is $-, 9;
  is $^, "STDOUT_DUP_TOP";
}
select $oldfh;
close STDOUT_DUP;

*CmT =  *{$::{Comment}}{FORMAT};
ok  defined *{$::{CmT}}{FORMAT}, "glob assign";


# RT #91032: Check that "non-real" strings like tie and overload work,
# especially that they re-compile the pattern on each FETCH, and that
# they don't overrun the buffer


{
    package RT91032;

    sub TIESCALAR { bless [] }
    my $i = 0;
    sub FETCH { $i++; "A$i @> Z\n" }

    use overload '""' => \&FETCH;

    tie my $f, 'RT91032';

    formline $f, "a";
    formline $f, "bc";
    ::is $^A, "A1  a Z\nA2 bc Z\n", "RT 91032: tied";
    $^A = '';

    my $g = bless []; # has overloaded stringify
    formline $g, "de";
    formline $g, "f";
    ::is $^A, "A3 de Z\nA4  f Z\n", "RT 91032: overloaded";
    $^A = '';

    my $h = [];
    formline $h, "junk1";
    formline $h, "junk2";
    ::is ref($h), 'ARRAY', "RT 91032: array ref still a ref";
    ::like "$h", qr/^ARRAY\(0x[0-9a-f]+\)$/, "RT 91032: array stringifies ok";
    ::is $^A, "$h$h","RT 91032: stringified array";
    $^A = '';

    # used to overwrite the ~~ in the *original SV with spaces. Naughty!

    my $orig = my $format = "^<<<<< ~~\n";
    my $abc = "abc";
    formline $format, $abc;
    $^A ='';
    ::is $format, $orig, "RT91032: don't overwrite orig format string";

    # check that ~ and ~~ are displayed correctly as whitespace,
    # under the influence of various different types of border

    for my $n (1,2) {
	for my $lhs (' ', 'Y', '^<<<', '^|||', '^>>>') {
	    for my $rhs ('', ' ', 'Z', '^<<<', '^|||', '^>>>') {
		my $fmt = "^<B$lhs" . ('~' x $n) . "$rhs\n";
		my $sfmt = ($fmt =~ s/~/ /gr);
		my ($a, $bc, $stop);
		($a, $bc, $stop) = ('a', 'bc', 's');
		# $stop is to stop '~~' deleting the whole line
		formline $sfmt, $stop, $a, $bc;
		my $exp = $^A;
		$^A = '';
		($a, $bc, $stop) = ('a', 'bc', 's');
		formline $fmt, $stop, $a, $bc;
		my $got = $^A;
		$^A = '';
		$fmt =~ s/\n/\\n/;
		::is($got, $exp, "chop munging: [$fmt]");
	    }
	}
    }
}

# check that '~  (delete current line if empty) works when
# the target gets upgraded to uft8 (and re-allocated) midstream.

{
    my $format = "\x{100}@~\n"; # format is utf8
    # this target is not utf8, but will expand (and get reallocated)
    # when upgraded to utf8.
    my $orig = "\x80\x81\x82";
    local $^A = $orig;
    my $empty = "";
    formline $format, $empty;
    is $^A , $orig, "~ and realloc";

    # check similarly that trailing blank removal works ok

    $format = "@<\n\x{100}"; # format is utf8
    chop $format;
    $orig = "   ";
    $^A = $orig;
    formline $format, "  ";
    is $^A, "$orig\n", "end-of-line blanks and realloc";

    # and check this doesn't overflow the buffer

    local $^A = '';
    $format = "@* @####\n";
    $orig = "x" x 100 . "\n";
    formline $format, $orig, 12345;
    is $^A, ("x" x 100) . " 12345\n", "\@* doesn't overflow";

    # ...nor this (RT #130703).
    # Under 'use bytes', the two bytes (c2, 80) making up each \x80 char
    # each get expanded to two bytes (so four in total per \x80 char); the
    # buffer growth wasn't accounting for this doubling in size

    {
        local $^A = '';
        my $format = "X\n\x{100}" . ("\x80" x 200);
        my $expected = $format;
        utf8::encode($expected);
        use bytes;
        formline($format);
        is $^A, $expected, "RT #130703";
    }

    # further buffer overflows with RT #130703

    {
        local $^A = '';
        my $n = 200;
        my $long = 'x' x 300;
        my $numf = ('@###' x $n);
        my $expected = $long . "\n" . ("   1" x $n);
        formline("@*\n$numf", $long, ('1') x $n);

        is $^A, $expected, "RT #130703 part 2";
    }


    # make sure it can cope with formats > 64k

    $format = 'x' x 65537;
    $^A = '';
    formline $format;
    # don't use 'is' here, as the diag output will be too long!
    ok $^A eq $format, ">64K";
}


SKIP: {
    skip_if_miniperl('miniperl does not support scalario');
    my $buf = "";
    open my $fh, ">", \$buf;
    my $old_fh = select $fh;
    local $~ = "CmT";
    write;
    select $old_fh;
    close $fh;
    is $buf, "ok $test\n", "write to duplicated format";
}

format caret_A_test_TOP =
T
.

format caret_A_test =
L1
L2
L3
L4
.

SKIP: {
    skip_if_miniperl('miniperl does not support scalario');
    my $buf = "";
    open my $fh, ">", \$buf;
    my $old_fh = select $fh;
    local $^ = "caret_A_test_TOP";
    local $~ = "caret_A_test";
    local $= = 3;
    local $^A = "A1\nA2\nA3\nA4\n";
    write;
    select $old_fh;
    close $fh;
    is $buf, "T\nA1\nA2\n\fT\nA3\nA4\n\fT\nL1\nL2\n\fT\nL3\nL4\n",
		    "assign to ^A sets FmLINES";
}

fresh_perl_like(<<'EOP', qr/^Format STDOUT redefined at/, {stderr => 1}, '#64562 - Segmentation fault with redefined formats and warnings');
#!./perl

use strict;
use warnings; # crashes!

format =
.

write;

format =
.

write;
EOP

fresh_perl_is(<<'EOP', ">ARRAY<\ncrunch_eth\n", {stderr => 1}, '#79532 - formline coerces its arguments');
use strict;
use warnings;
my $zamm = ['crunch_eth'];
formline $zamm;
printf ">%s<\n", ref $zamm;
print "$zamm->[0]\n";
EOP

# [perl #129125] - detected by -fsanitize=address or valgrind
# the compiled format would be freed when the format string was modified
# by the chop operator
fresh_perl_is(<<'EOP', "^", { stderr => 1 }, '#129125 - chop on format');
my $x = '^@';
formline$x=>$x;
print $^A;
EOP

fresh_perl_is(<<'EOP', '<^< xx AA><xx ^<><>', { stderr => 1 }, '#129125 - chop on format, later values');
my $x = '^< xx ^<';
my $y = 'AA';
formline $x => $x, $y;
print "<$^A><$x><$y>";
EOP


# [perl #73690]

select +(select(RT73690), do {
    open(RT73690, '>Op_write.tmp') || die "Can't create Op_write.tmp";
    format RT73690 =
@<< @<<
11, 22
.

    my @ret;

    @ret = write;
    is(scalar(@ret), 1);
    ok($ret[0]);
    @ret = scalar(write);
    is(scalar(@ret), 1);
    ok($ret[0]);
    @ret = write(RT73690);
    is(scalar(@ret), 1);
    ok($ret[0]);
    @ret = scalar(write(RT73690));
    is(scalar(@ret), 1);
    ok($ret[0]);

    @ret = ('a', write, 'z');
    is(scalar(@ret), 3);
    is($ret[0], 'a');
    ok($ret[1]);
    is($ret[2], 'z');
    @ret = ('b', scalar(write), 'y');
    is(scalar(@ret), 3);
    is($ret[0], 'b');
    ok($ret[1]);
    is($ret[2], 'y');
    @ret = ('c', write(RT73690), 'x');
    is(scalar(@ret), 3);
    is($ret[0], 'c');
    ok($ret[1]);
    is($ret[2], 'x');
    @ret = ('d', scalar(write(RT73690)), 'w');
    is(scalar(@ret), 3);
    is($ret[0], 'd');
    ok($ret[1]);
    is($ret[2], 'w');

    @ret = do { write; 'foo' };
    is(scalar(@ret), 1);
    is($ret[0], 'foo');
    @ret = do { scalar(write); 'bar' };
    is(scalar(@ret), 1);
    is($ret[0], 'bar');
    @ret = do { write(RT73690); 'baz' };
    is(scalar(@ret), 1);
    is($ret[0], 'baz');
    @ret = do { scalar(write(RT73690)); 'quux' };
    is(scalar(@ret), 1);
    is($ret[0], 'quux');

    @ret = ('a', do { write; 'foo' }, 'z');
    is(scalar(@ret), 3);
    is($ret[0], 'a');
    is($ret[1], 'foo');
    is($ret[2], 'z');
    @ret = ('b', do { scalar(write); 'bar' }, 'y');
    is(scalar(@ret), 3);
    is($ret[0], 'b');
    is($ret[1], 'bar');
    is($ret[2], 'y');
    @ret = ('c', do { write(RT73690); 'baz' }, 'x');
    is(scalar(@ret), 3);
    is($ret[0], 'c');
    is($ret[1], 'baz');
    is($ret[2], 'x');
    @ret = ('d', do { scalar(write(RT73690)); 'quux' }, 'w');
    is(scalar(@ret), 3);
    is($ret[0], 'd');
    is($ret[1], 'quux');
    is($ret[2], 'w');

    close RT73690 or die "Could not close: $!";
})[0];

select +(select(RT73690_2), do {
    open(RT73690_2, '>Op_write.tmp') || die "Can't create Op_write.tmp";
    format RT73690_2 =
@<< @<<
return
.

    my @ret;

    @ret = write;
    is(scalar(@ret), 1);
    ok(!$ret[0]);
    @ret = scalar(write);
    is(scalar(@ret), 1);
    ok(!$ret[0]);
    @ret = write(RT73690_2);
    is(scalar(@ret), 1);
    ok(!$ret[0]);
    @ret = scalar(write(RT73690_2));
    is(scalar(@ret), 1);
    ok(!$ret[0]);

    @ret = ('a', write, 'z');
    is(scalar(@ret), 3);
    is($ret[0], 'a');
    ok(!$ret[1]);
    is($ret[2], 'z');
    @ret = ('b', scalar(write), 'y');
    is(scalar(@ret), 3);
    is($ret[0], 'b');
    ok(!$ret[1]);
    is($ret[2], 'y');
    @ret = ('c', write(RT73690_2), 'x');
    is(scalar(@ret), 3);
    is($ret[0], 'c');
    ok(!$ret[1]);
    is($ret[2], 'x');
    @ret = ('d', scalar(write(RT73690_2)), 'w');
    is(scalar(@ret), 3);
    is($ret[0], 'd');
    ok(!$ret[1]);
    is($ret[2], 'w');

    @ret = do { write; 'foo' };
    is(scalar(@ret), 1);
    is($ret[0], 'foo');
    @ret = do { scalar(write); 'bar' };
    is(scalar(@ret), 1);
    is($ret[0], 'bar');
    @ret = do { write(RT73690_2); 'baz' };
    is(scalar(@ret), 1);
    is($ret[0], 'baz');
    @ret = do { scalar(write(RT73690_2)); 'quux' };
    is(scalar(@ret), 1);
    is($ret[0], 'quux');

    @ret = ('a', do { write; 'foo' }, 'z');
    is(scalar(@ret), 3);
    is($ret[0], 'a');
    is($ret[1], 'foo');
    is($ret[2], 'z');
    @ret = ('b', do { scalar(write); 'bar' }, 'y');
    is(scalar(@ret), 3);
    is($ret[0], 'b');
    is($ret[1], 'bar');
    is($ret[2], 'y');
    @ret = ('c', do { write(RT73690_2); 'baz' }, 'x');
    is(scalar(@ret), 3);
    is($ret[0], 'c');
    is($ret[1], 'baz');
    is($ret[2], 'x');
    @ret = ('d', do { scalar(write(RT73690_2)); 'quux' }, 'w');
    is(scalar(@ret), 3);
    is($ret[0], 'd');
    is($ret[1], 'quux');
    is($ret[2], 'w');

    close RT73690_2 or die "Could not close: $!";
})[0];

open(UNDEF, '>Op_write.tmp') || die "Can't create Op_write.tmp";
select +(select(UNDEF), $~ = "UNDEFFORMAT")[0];
format UNDEFFORMAT =
@
undef *UNDEFFORMAT
.
write UNDEF;
pass "active format cannot be freed";

select +(select(UNDEF), $~ = "UNDEFFORMAT2")[0];
format UNDEFFORMAT2 =
@
close UNDEF or die "Could not close: $!"; undef *UNDEF
.
write UNDEF;
pass "freeing current handle in format";
undef $^A;

ok !eval q|
format foo {
@<<<
$a
}
;1
|, 'format foo { ... } is not allowed';

ok !eval q|
format =
@<<<
}
;1
|, 'format = ... } is not allowed';

open(NEST, '>Op_write.tmp') || die "Can't create Op_write.tmp";
format NEST =
@<<<
{
    my $birds = "birds";
    local *NEST = *BIRDS{FORMAT};
    write NEST;
    format BIRDS =
@<<<<<
$birds;
.
    "nest"
}
.
write NEST;
close NEST or die "Could not close: $!";
is cat('Op_write.tmp'), "birds\nnest\n", 'nested formats';

# A compilation error should not create a format
eval q|
format ERROR =
@
@_ =~ s///
.
|;
eval { write ERROR };
like $@, qr'Undefined format',
    'formats with compilation errors are not created';

# This syntax error used to cause a crash, double free, or a least
# a bad read.
# See the long-winded explanation at:
#   https://github.com/Perl/perl5/issues/8953#issuecomment-543978716
eval q|
format =
@
use;format
strict
.
|;
pass('no crash with invalid use/format inside format');


# Low-precedence operators on argument line
format AND =
@
0 and die
.
$- = $=;
ok eval { local $~ = "AND"; print "# "; write; 1 },
    "low-prec ops on arg line" or diag $@;

# Anonymous hashes
open(HASH, '>Op_write.tmp') || die "Can't create Op_write.tmp";
format HASH =
@<<<
${{qw[ Sun 0 Mon 1 Tue 2 Wed 3 Thu 4 Fri 5 Sat 6 ]}}{"Wed"}
.
write HASH;
close HASH or die "Could not close: $!";
is cat('Op_write.tmp'), "3\n", 'anonymous hashes';

open(HASH2, '>Op_write.tmp') || die "Can't create Op_write.tmp";
format HASH2 =
@<<<
+{foo=>"bar"}
.
write HASH2;
close HASH2 or die "Could not close: $!";
is cat('Op_write.tmp'), "HASH\n", '+{...} is interpreted as anon hash';

# Anonymous hashes
open(BLOCK, '>Op_write.tmp') || die "Can't create Op_write.tmp";
format BLOCK =
@<<< @<<<
{foo=>"bar"} # this is a block, not a hash!
.
write BLOCK;
close BLOCK or die "Could not close: $!";
is cat('Op_write.tmp'), "foo  bar\n", 'initial { is always BLOCK';

# pragmata inside argument line
open(STRICT, '>Op_write.tmp') || die "Can't create Op_write.tmp";
format STRICT =
@<<<
no strict; $foo
.
$::foo = 'oof::$';
write STRICT;
close STRICT or die "Could not close: $!";
is cat('Op_write.tmp'), "oof:\n", 'pragmata on format line';

{
   no warnings 'experimental::builtin';
   use builtin 'weaken';
   sub Potshriggley {
format Potshriggley =
.
   }
   weaken(my $x = *Potshriggley{FORMAT});
   undef *Potshriggley;
   is $x, undef, 'formats in subs do not leak';
}

fresh_perl_is(<<'EOP', <<'EXPECT',
use warnings 'syntax' ;
format STDOUT =
^*|^*
my $x = q/dd/, $x
.
write;
EOP
dd|
EXPECT
	      { stderr => 1 }, '#123245 panic in sv_chop');

fresh_perl_is(<<'EOP', <<'EXPECT',
use warnings 'syntax' ;
format STDOUT =
^*|^*
my $x = q/dd/
.
write;
EOP
Not enough format arguments at - line 4.
dd|
EXPECT
	      { stderr => 1 }, '#123245 different panic in sv_chop');

fresh_perl_is(<<'EOP', <<'EXPECT',
format STDOUT =
# x at the end to make the spaces visible
@... x
q/a/
.
write;
EOP
a    x
EXPECT
	      { stderr => 1 }, '#123538 crash in FF_MORE');

{
    $^A = "";
    my $a = *globcopy;
    my $r = eval { formline "^<<", $a };
    is $@, "";
    ok $r, "^ format with glob copy";
    is $^A, "*ma", "^ format with glob copy";
    is $a, "in::globcopy", "^ format with glob copy";
}

{
    $^A = "";
    my $r = eval { formline "^<<", *realglob };
    like $@, qr/\AModification of a read-only value attempted /;
    is $r, undef, "^ format with real glob";
    is $^A, "*ma", "^ format with real glob";
    is ref(\*realglob), "GLOB";
}

$^A = "";

# [perl #130722] assertion failure
fresh_perl_is('for(1..2){formline*0}', '', { stderr => 1 } , "#130722 - assertion failure");

#############################
## Section 4
## Add new tests *above* here
#############################

# scary format testing from H.Merijn Brand

# Just a complete test for format, including top-, left- and bottom marging
# and format detection through glob entries

if ($^O eq 'VMS' || $^O eq 'MSWin32' ||
    ($^O eq 'os2' and not eval '$OS2::can_fork')) {
  $test = curr_test();
 SKIP: {
      skip "'|-' and '-|' not supported", $tests - $test + 1;
  }
  exit(0);
}


$^  = "STDOUT_TOP";
$=  =  7;		# Page length
$-  =  0;		# Lines left
my $ps = $^L; $^L = "";	# Catch the page separator
my $tm =  1;		# Top margin (empty lines before first output)
my $bm =  2;		# Bottom marging (empty lines between last text and footer)
my $lm =  4;		# Left margin (indent in spaces)

# -----------------------------------------------------------------------
#
# execute the rest of the script in a child process. The parent reads the
# output from the child and compares it with <DATA>.

my @data = <DATA>;

select ((select (STDOUT), $| = 1)[0]); # flush STDOUT

my $opened = open FROM_CHILD, "-|";
unless (defined $opened) {
    fail "open gave $!";
    exit 0;
}

if ($opened) {
    # in parent here

    pass 'open';
    my $s = " " x $lm;
    while (<FROM_CHILD>) {
	unless (@data) {
	    fail 'too much output';
	    exit;
	}
	s/^/$s/;
	my $exp = shift @data;
	is $_, $exp;
    }
    close FROM_CHILD;
    is "@data", "", "correct length of output";
    exit;
}

# in child here
$::NO_ENDING = 1;

    select ((select (STDOUT), $| = 1)[0]);
$tm = "\n" x $tm;
$= -= $bm + 1; # count one for the trailing "----"
my $lastmin = 0;

my @E;

sub wryte
{
    $lastmin = $-;
    write;
    } # wryte;

sub footer
{
    $% == 1 and return "";

    $lastmin < $= and print "\n" x $lastmin;
    print "\n" x $bm, "----\n", $ps;
    $lastmin = $-;
    "";
    } # footer

# Yes, this is sick ;-)
format TOP =
@* ~
@{[footer]}
@* ~
$tm
.

format ENTRY =
@ @<<<<~~
@{(shift @E)||["",""]}
.

format EOR =
- -----
.

sub has_format ($)
{
    my $fmt = shift;
    exists $::{$fmt} or return 0;
    $^O eq "MSWin32" or return defined *{$::{$fmt}}{FORMAT};
    open my $null, "> /dev/null" or die;
    my $fh = select $null;
    local $~ = $fmt;
    eval "write";
    select $fh;
    $@?0:1;
    } # has_format

$^ = has_format ("TOP") ? "TOP" : "EMPTY";
has_format ("ENTRY") or die "No format defined for ENTRY";
foreach my $e ( [ map { [ $_, "Test$_"   ] } 1 .. 7 ],
		[ map { [ $_, "${_}tseT" ] } 1 .. 5 ]) {
    @E = @$e;
    local $~ = "ENTRY";
    wryte;
    has_format ("EOR") or next;
    local $~ = "EOR";
    wryte;
    }
if (has_format ("EOF")) {
    local $~ = "EOF";
    wryte;
    }

close STDOUT;

# That was test 48.

__END__
    
    1 Test1
    2 Test2
    3 Test3
    
    
    ----
    
    4 Test4
    5 Test5
    6 Test6
    
    
    ----
    
    7 Test7
    - -----
    
    
    
    ----
    
    1 1tseT
    2 2tseT
    3 3tseT
    
    
    ----
    
    4 4tseT
    5 5tseT
    - -----
