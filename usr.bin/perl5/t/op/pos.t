#!./perl

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}

plan tests => 33;

$x='banana';
$x=~/.a/g;
is(pos($x), 2, "matching, pos() leaves off at offset 2");

$x=~/.z/gc;
is(pos($x), 2, "not matching, pos() remains at offset 2");

sub f { my $p=$_[0]; return $p }

$x=~/.a/g;
is(f(pos($x)), 4, "matching again, pos() next leaves off at offset 4");

# Is pos() set inside //g? (bug id 19990615.008 (#874))
$x = "test string?"; $x =~ s/\w/pos($x)/eg;
is($x, "0123 5678910?", "pos() set inside //g");

$x = "123 56"; $x =~ / /g;
is(pos($x), 4, "matching, pos() leaves off at offset 4");
{ local $x }
is(pos($x), 4, "value of pos() unaffected by intermediate localization");

# Explicit test that triggers the utf8_mg_len_cache_update() code path in
# Perl_sv_pos_b2u().

$x = "\x{100}BC";
$x =~ m/.*/g;
is(pos $x, 3, "utf8_mg_len_cache_update() test");

is(scalar pos $x, 3, "rvalue pos() utf8 test");


my $destroyed;
{ package Class; DESTROY { ++$destroyed; } }

$destroyed = 0;
{
    my $x = '';
    pos($x) = 0;
    $x = bless({}, 'Class');
}
is($destroyed, 1, 'Timely scalar destruction with lvalue pos');

eval 'pos @a = 1';
like $@, qr/^Can't modify array dereference in match position at /,
  'pos refuses @arrays';
eval 'pos %a = 1';
like $@, qr/^Can't modify hash dereference in match position at /,
  'pos refuses %hashes';
eval 'pos *a = 1';
is eval 'pos *a', 1, 'pos *glob works';

# Test that UTF8-ness of $1 changing does not confuse pos
"f" =~ /(f)/; "$1";	# first make sure UTF8-ness is off
"\x{100}a" =~ /(..)/;	# give PL_curpm a UTF8 string; $1 does not know yet
pos($1) = 2;		# set pos; was ignoring UTF8-ness
"$1";			# turn on UTF8 flag
is pos($1), 2, 'pos is not confused about changing UTF8-ness';

sub {
    $_[0] = "hello";
    pos $_[0] = 3;
    is pos $h{k}, 3, 'defelems can propagate pos assignment';
    $_[0] =~ /./g;
    is pos $h{k}, 4, 'defelems can propagate implicit pos (via //g)';
    $_[0] =~ /oentuhoetn/g;
    is pos $h{k}, undef, 'failed //g sets pos through defelem';
    $_[1] = "hello";
    pos $h{l} = 3;
    is pos $_[1], 3, 'reading pos through a defelem';
    pos $h{l} = 4;
    $_[1] =~ /(.)/g;
    is "$1", 'o', '//g can read pos through a defelem';
    $_[2] = "hello";
    () = $_[2] =~ /l/gc;
    is pos $h{m}, 4, '//gc in list cx can set pos through a defelem';
    $_[3] = "hello";
    $_[3] =~
        s<e><is pos($h{n}), 1, 's///g setting pos through a defelem'>egg;
    $h{n} = 'hello';
    $_[3] =~ /e(?{ is pos $h{n},2, 're-evals set pos through defelems' })/;
    pos $h{n} = 1;
    ok $_[3] =~ /\Ge/, '\G works with defelem scalars';
}->($h{k}, $h{l}, $h{m}, $h{n});

$x = bless [], chr 256;
pos $x=1;
bless $x, a;
is pos($x), 1, 'pos is not affected by reference stringification changing';
{
    my $w;
    local $SIG{__WARN__} = sub { $w .= shift };
    $x = bless [], chr 256;
    pos $x=1;
    bless $x, "\x{1000}";
    is pos $x, 1,
       'pos unchanged after increasing size of chars in stringification';
    is $w, undef, 'and no malformed utf8 warning';
}
$x = bless [], chr 256;
$x =~ /.(?{
     bless $x, a;
     is pos($x), 1, 'pos unaffected by ref str changing (in re-eval)';
})/;
{
    my $w;
    local $SIG{__WARN__} = sub { $w .= shift };
    $x = bless [], chr(256);
    $x =~ /.(?{
        bless $x, "\x{1000}";
        is pos $x, 1,
         'pos unchanged in re-eval after increasing size of chars in str';
    })/;
    is $w, undef, 'and no malformed utf8 warning';
}

for my $one(pos $x) {
    for my $two(pos $x) {
        $one = \1;
        $two = undef;
        is $one, undef,
           'no assertion failure when getting pos clobbers ref with undef';
    }
}

{
    # RT # 127518
    my $x = "\N{U+10000}abc";
    my %expected = (
        chars   => { length => 4, pos => 2 },
        bytes   => { length => 7, pos => 5 },
    );
    my %observed;
    $observed{chars}{length} = length($x);
    $x =~ m/a/g;
    $observed{chars}{pos}    = pos($x);

    {
        use bytes;
        $observed{bytes}{length} = length($x);
        $observed{bytes}{pos}    = pos($x);
    }

    is( $observed{chars}{length}, $expected{chars}{length},
         "Got expected length in chars");
    is( $observed{chars}{pos}, $expected{chars}{pos},
         "Got expected pos in chars");
    is( $observed{bytes}{length}, $expected{bytes}{length},
         "Got expected length in bytes");
    is( $observed{bytes}{pos}, $expected{bytes}{pos},
         "Got expected pos in bytes");
}
