#!./perl

# Test that $lexical = <some op> optimises the assignment away correctly
# and causes no ill side-effects.

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}

$| = 1;
umask 0;
$xref = \ "";
$runme = $^X;
@a = (1..5);
%h = (1..6);
$aref = \@a;
$href = \%h;
open OP, qq{$runme -le "print 'aaa Ok ok' for 1..100"|};
$chopit = 'aaaaaa';
@chopar = (113 .. 119);
$posstr = '123456';
$cstr = 'aBcD.eF';
pos $posstr = 3;
$nn = $n = 2;
sub subb {"in s"}

@INPUT = <DATA>;
@simple_input = grep /^\s*\w+\s*\$\w+\s*[#\n]/, @INPUT;

sub wrn {"@_"}

# Check correct optimization of ucfirst etc
my $a = "AB";
my $b = "\u\L$a";
is( $b, 'Ab', 'Check correct optimization of ucfirst, etc');

# Check correct destruction of objects:
my $dc = 0;
sub A::DESTROY {$dc += 1}
$a=8;
my $b;
{ my $c = 6; $b = bless \$c, "A"}

is($dc, 0, 'No destruction yet');

$b = $a+5;

is($dc, 1, 'object destruction via reassignment to variable');

my $xxx = 'b';
$xxx = 'c' . ($xxx || 'e');
is( $xxx, 'cb', 'variables can be read before being overwritten');

# Chains of assignments

my ($l1, $l2, $l3, $l4);
my $zzzz = 12;
$zzz1 = $l1 = $l2 = $zzz2 = $l3 = $l4 = 1 + $zzzz;

is($zzz1, 13, 'chain assignment, part1');
is($zzz2, 13, 'chain assignment, part2');
is($l1,   13, 'chain assignment, part3');
is($l2,   13, 'chain assignment, part4');
is($l3,   13, 'chain assignment, part5');
is($l4,   13, 'chain assignment, part6');

for (@INPUT) {
  ($op, undef, $comment) = /^([^\#]+)(\#\s+(.*))?/;
  $comment = $op unless defined $comment;
  chomp;
  $op = "$op==$op" unless $op =~ /==/;
  ($op, $expectop) = $op =~ /(.*)==(.*)/;
  
  $skip = ($op =~ /^'\?\?\?'/ or $comment =~ /skip\(.*\Q$^O\E.*\)/i);
  $integer = ($comment =~ /^i_/) ? "use integer" : '' ;
  if ($skip) {
    SKIP: {
        skip $comment, 1;
    }
    next;
  }
  
  eval <<EOE;
  local \$SIG{__WARN__} = \\&wrn;
  my \$a = 'fake';
  $integer;
  \$a = $op;
  \$b = $expectop;
  is (\$a, \$b, \$comment);
EOE
  if ($@) {
    $warning = $@;
    chomp $warning;
    if ($@ !~ /(?:is un|not )implemented/) {
      fail($_ . ' ' . $warning);
    }
  }
}

{				# Check calling STORE
  note('Tied variables, calling STORE');
  my $sc = 0;
  # do not use B:: namespace
  sub BB::TIESCALAR {bless [11], 'BB'}
  sub BB::FETCH { -(shift->[0]) }
  sub BB::STORE { $sc++; my $o = shift; $o->[0] = 17 + shift }

  my $m;
  tie $m, 'BB';
  $m = 100;

  is( $sc, 1, 'STORE called when assigning scalar to tied variable' );

  my $t = 11;
  $m = $t + 89;
  
  is( $sc, 2, 'and again' );
  is( $m,  -117, 'checking the tied variable result' );

  $m += $t;

  is( $sc, 3, 'called on self-increment' );
  is( $m,  89, 'checking the tied variable result' );

  for (@INPUT) {
    ($op, undef, $comment) = /^([^\#]+)(\#\s+(.*))?/;
    $comment = $op unless defined $comment;
    next if ($op =~ /^'\?\?\?'/ or $comment =~ /skip\(.*\Q$^O\E.*\)/i);
    $op =~ s/==.*//;
    
    $sc = 0;
    local $SIG{__WARN__} = \&wrn;
    eval "\$m = $op";
    is $sc, $@ ? 0 : 1, "STORE count for $comment";
  }
}

for (@simple_input) {
  ($op, undef, $comment) = /^([^\#]+)(\#\s+(.*))?/;
  $comment = $op unless defined $comment;
  chomp;
  ($operator, $variable) = /^\s*(\w+)\s*\$(\w+)/ or warn "misprocessed '$_'\n";
  eval <<EOE;
  local \$SIG{__WARN__} = \\&wrn;
  my \$$variable = "Ac# Ca\\nxxx";
  \$$variable = $operator \$$variable;
  \$toself = \$$variable;
  \$direct = $operator "Ac# Ca\\nxxx";
  is(\$toself, \$direct);
EOE
  if ($@) {
    $warning = $@;
    chomp $warning;
    if ($@ =~ /(?:is un|not )implemented/) {
      SKIP: {
        skip $warning, 1;
        pass($comment);
      }
    } elsif ($@ =~ /Can't (modify|take log of 0)/) {
      SKIP: {
        skip $warning . ' ' . $comment . ' syntax not good for selfassign', 1;
        pass();
      }
    } else {
      ##Something bad happened
      fail($_ . ' ' . $warning);
    }
  }
}

# [perl #123790] Assigning to a typeglob
# These used to die or crash.
# Once the bug is fixed for all ops, we can combine this with the tests
# above that use <DATA>.
for my $glob (*__) {
  $glob = $y x $z;
  { use integer; $glob = $y <=> $z; }
  $glob = $y cmp $z;
  $glob = vec 1, 2, 4;
  $glob = ~${\""};
  $glob = split;
}

# XXX This test does not really belong here, as it has nothing to do with
#     OPpTARGET_MY optimisation.  But where should it go?
eval {
    sub PVBM () { 'foo' }
    index 'foo', PVBM;
    my $x = PVBM;

    my $str = 'foo';
    my $pvlv = \substr $str, 0, 1;
    $x = $pvlv;

    1;
};
is($@, '', 'ex-PVBM assert'.$@);

# RT perl #127855
# Check that stringification and assignment to itself doesn't break
# anything. This is unlikely to actually fail the tests; its more something
# for valgrind to spot. It will also only fail if SvGROW or its caller
# decides to over-allocate (otherwise copying the string will skip the
# sv_grow(), as the new size is the same as the current size).

{
    my $s;
    for my $len (1..40) {
        $s = 'x' x $len;
        my $t = $s;
        $t = "$t";
        ok($s eq $t, "RT 127855: len=$len");
    }
}

# time() can't be tested using the standard framework since two successive
# calls may return differing values.

{
    my $a;
    $a = time;
    $b = time;
    my $diff = $b - $a;
    cmp_ok($diff, '>=', 0,  "time is monotically increasing");
    cmp_ok($diff, '<',  2,  "time delta is small");
}

# GH #20132 and parts of GH ##20114
# During development of OP_PADSV_STORE, interactions with OP_PADRANGE
# caused BBC failures not picked up by any pre-existing core tests.
# (Problems only arose in list context, the void/scalar tests have been
# included for completeness.)
eval {
    my $x = {}; my $y;
    keys %{$y = $x};
    1;
};
is($@, '', 'keys %{$y = $x}');

eval {
    my $x = {}; my $y;
    my $foo = keys %{$y = $x};
    1;
};
is($@, '', 'my $foo = keys %{$y = $x}');

eval {
    my $x = {}; my $y;
    my @foo = keys %{$y = $x};
    1;
};
is($@, '', 'my @foo = keys %{$y = $x}');

fresh_perl_is('my ($x, $y); (($y = $x))', '', {}, '(($y = $x))');
fresh_perl_is('my ($x, $y); my $z= (($y = $x))', '', {}, 'my $z= (($y = $x))');
fresh_perl_is('my ($x, $y); my @z= (($y = $x))', '', {}, 'my @z= (($y = $x))');

done_testing();

__END__
ref $xref			# ref
ref $cstr			# ref nonref
`$runme -e "print qq[1\\n]"`				# backtick skip(MSWin32)
`$undefed`			# backtick undef skip(MSWin32)
'???'				# glob  (not currently OA_TARGLEX)
<OP>				# readline
'faked'				# rcatline
(@z = (1 .. 3))			# aassign
(chop (@x=@chopar))		# chop
chop $chopit			# schop
(chomp (@x=@chopar))		# chomp
chomp $chopit			# schomp
pos $posstr			# pos
pos $chopit			# pos returns undef
$nn++==2			# postinc
$nn++==3			# i_postinc
$nn--==4			# postdec
$nn--==3			# i_postdec
$n ** $n			# pow
$n * $n				# multiply
$n * $n				# i_multiply
$n / $n				# divide
$n / $n				# i_divide
$n % $n				# modulo
$n % $n				# i_modulo
$n x $n				# repeat
$n + $n				# add
$n + $n				# i_add
$n - $n				# subtract
$n - $n				# i_subtract
$n . $n				# concat
$n . $a=='2fake'		# concat with self
"3$a"=='3fake'			# concat with self in stringify
"$n"				# stringify
$n << $n			# left_shift
$n >> $n			# right_shift
$n <=> $n			# ncmp
$n <=> $n			# i_ncmp
$n cmp $n			# scmp
$n & $n				# bit_and
$n ^ $n				# bit_xor
$n | $n				# bit_or
-$n				# negate
-$n				# i_negate
-$a=="-fake"			# i_negate with string
~$n				# complement
atan2 $n,$n			# atan2
sin $n				# sin
cos $n				# cos
'???'				# rand
exp $n				# exp
log $n				# log
sqrt $n				# sqrt
int $n				# int
hex $n				# hex
oct $n				# oct
abs $n				# abs
length $posstr			# length
substr $posstr, 2, 2		# substr
vec("abc",2,8)			# vec
index $posstr, 2		# index
rindex $posstr, 2		# rindex
sprintf "%i%i", $n, $n		# sprintf
ord $n				# ord
chr $n				# chr
chr ${\256}			# chr $wide
crypt $n, $n			# crypt
ucfirst ($cstr . "a")		# ucfirst padtmp
ucfirst $cstr			# ucfirst
lcfirst $cstr			# lcfirst
uc $cstr			# uc
lc $cstr			# lc
quotemeta $cstr			# quotemeta
@$aref				# rv2av
@$undefed			# rv2av undef
(each %h) % 2 == 1		# each
values %h			# values
keys %h				# keys
%$href				# rv2hv
pack "C2", $n,$n		# pack
split /a/, "abad"		# split
join "a"; @a			# join
push @a,3==6			# push
unshift @aaa			# unshift
reverse	@a			# reverse
reverse	$cstr			# reverse - scal
grep $_, 1,0,2,0,3		# grepwhile
map "x$_", 1,0,2,0,3		# mapwhile
subb()				# entersub
caller				# caller
warn "ignore this\n"		# warn
'faked'				# die
open BLAH, "<non-existent"	# open
fileno STDERR			# fileno
umask 0				# umask
select STDOUT			# sselect
select undef,undef,undef,0	# select
getc OP				# getc
'???'				# read
'???'				# sysread
'???'				# syswrite
'???'				# send
'???'				# recv
'???'				# tell
'???'				# fcntl
'???'				# ioctl
'???'				# flock
'???'				# accept
'???'				# shutdown
'???'				# ftsize
'???'				# ftmtime
'???'				# ftatime
'???'				# ftctime
chdir 'non-existent'		# chdir
'???'				# chown
'???'				# chroot
unlink 'non-existent'		# unlink
chmod 'non-existent'		# chmod
utime 'non-existent'		# utime
rename 'non-existent', 'non-existent1'	# rename
link 'non-existent', 'non-existent1' # link
'???'				# symlink
readlink 'non-existent', 'non-existent1' # readlink
'???'				# mkdir
'???'				# rmdir
'???'				# telldir
'???'				# fork
'???'				# wait
'???'				# waitpid
system "$runme -e 0"		# system skip(VMS)
'???'				# exec
'???'				# kill
getppid				# getppid
getpgrp				# getpgrp
setpgrp				# setpgrp
getpriority $$, $$		# getpriority
'???'				# setpriority
'???'				# time
localtime $^T			# localtime
gmtime $^T			# gmtime
'???'				# sleep: can randomly fail
'???'				# alarm
'???'				# shmget
'???'				# shmctl
'???'				# shmread
'???'				# shmwrite
'???'				# msgget
'???'				# msgctl
'???'				# msgsnd
'???'				# msgrcv
'???'				# semget
'???'				# semctl
'???'				# semop
'???'				# getlogin
'???'				# syscall
