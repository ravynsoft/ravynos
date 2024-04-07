#!./perl -w

select(STDERR); $| = 1;
select(STDOUT); $| = 1;

print "1..27\n";

use IO::Select 1.09;

my $sel = IO::Select->new(\*STDIN);
$sel->add(4, 5) == 2 or print "not ";
print "ok 1\n";

$sel->add([\*STDOUT, 'foo']) == 1 or print "not ";
print "ok 2\n";

my @handles = $sel->handles;
print "not " unless $sel->count == 4 && @handles == 4;
print "ok 3\n";
#print $sel->as_string, "\n";

$sel->remove(\*STDIN) == 1 or print "not ";
print "ok 4\n",
;
$sel->remove(\*STDIN, 5, 6) == 1  # two of there are not present
  or print "not ";
print "ok 5\n";

print "not " unless $sel->count == 2;
print "ok 6\n";
#print $sel->as_string, "\n";

$sel->remove(1, 4);
print "not " unless $sel->count == 0 && !defined($sel->bits);
print "ok 7\n";

$sel = IO::Select->new();
print "not " unless $sel->count == 0 && !defined($sel->bits);
print "ok 8\n";

$sel->remove([\*STDOUT, 5]);
print "not " unless $sel->count == 0 && !defined($sel->bits);
print "ok 9\n";

if ( grep $^O eq $_, qw(MSWin32 NetWare dos VMS riscos beos) ) {
    for (10 .. 15) { 
        print "ok $_ # skip: 4-arg select is only valid on sockets\n"
    }
    $sel->add(\*STDOUT);  # update
    goto POST_SOCKET;
}

my @a = $sel->can_read();  # should return immediately
print "not " unless @a == 0;
print "ok 10\n";

# we assume that we can write to STDOUT :-)
$sel->add([\*STDOUT, "ok 12\n"]);

@a = $sel->can_write;
print "not " unless @a == 1;
print "ok 11\n";

my($fd, $msg) = @{shift @a};
print $fd $msg;

$sel->add(\*STDOUT);  # update

@a = IO::Select::select(undef, $sel, undef, 1);
print "not " unless @a == 3;
print "ok 13\n";

my ($r, $w, $e) = @a;

print "not " unless @$r == 0 && @$w == 1 && @$e == 0;
print "ok 14\n";

$fd = $w->[0];
print $fd "ok 15\n";

POST_SOCKET:
# Test new exists() method
$sel->exists(\*STDIN) and print "not ";
print "ok 16\n";

($sel->exists(0) || $sel->exists([\*STDERR])) and print "not ";
print "ok 17\n";

$fd = $sel->exists(\*STDOUT);
if ($fd) {
    print $fd "ok 18\n";
} else {
    print "not ok 18\n";
}

$fd = $sel->exists([1, 'foo']);
if ($fd) {
    print $fd "ok 19\n";
} else {
    print "not ok 19\n";
}

# Try self clearing
$sel->add(5,6,7,8,9,10);
print "not " unless $sel->count == 7;
print "ok 20\n";

$sel->remove($sel->handles);
print "not " unless $sel->count == 0 && !defined($sel->bits);
print "ok 21\n";

# check warnings
$SIG{__WARN__} = sub { 
    ++ $w 
      if $_[0] =~ /^Call to deprecated method 'has_error', use 'has_exception'/ ;
    } ;
$w = 0 ;
{
no warnings 'IO::Select' ;
IO::Select::has_error();
}
print "not " unless $w == 0 ;
$w = 0 ;
print "ok 22\n" ;
{
use warnings 'IO::Select' ;
IO::Select::has_error();
}
print "not " unless $w == 1 ;
$w = 0 ;
print "ok 23\n" ;

{
    # perl #75156 - test we can delete a closed handle
    require IO::Socket::INET;
    my $fh = IO::Socket::INET->new(
      Listen => 5,
    );
    my $sel = IO::Select->new(\*STDIN);
    $sel->add($fh);
    my $oldbits = $sel->bits;
    print "not " unless $sel->count == 2;
    print "ok 24 - added socket\n";
    close $fh;
    print "not " unless $sel->remove($fh) == 1;
    print "ok 25 - removed closed socket\n";
    print "not " unless $sel->count == 1;
    print "ok 26 - count() updated\n";
    print "not " unless $sel->bits ne $oldbits;
    print "ok 27 - bits() updated\n";
}
