#!./perl

# We do all of the work in child processes here to ensure that any
# memory used is released immediately.

# These tests use ridiculous amounts of memory and CPU.

use strict;
use warnings;

use Config;
use Storable qw(store_fd retrieve_fd nstore_fd);
use Test::More;
use File::Temp qw(tempfile);
use File::Spec;

BEGIN {
    plan skip_all => 'Storable was not built'
        if $ENV{PERL_CORE} && $Config{'extensions'} !~ /\b Storable \b/x;
    plan skip_all => 'Need 64-bit pointers for this test'
        if $Config{ptrsize} < 8 and $] > 5.013;
    plan skip_all => 'Need 64-bit int for this test on older versions'
        if $Config{uvsize} < 8 and $] < 5.013;
    plan skip_all => 'Need ~8 GiB memory for this test, set PERL_TEST_MEMORY >= 8'
        if !$ENV{PERL_TEST_MEMORY} || $ENV{PERL_TEST_MEMORY} < 8;
    plan skip_all => 'These tests are slow, set PERL_RUN_SLOW_TESTS'
        unless $ENV{PERL_RUN_SLOW_TESTS};
    plan skip_all => "Need fork for this test",
        unless $Config{d_fork};
}

find_exe("gzip")
    or plan skip_all => "Need gzip for this test";
find_exe("gunzip")
    or plan skip_all => "Need gunzip for this test";

plan tests => 12;

my $skips = $ENV{PERL_STORABLE_SKIP_ID_TEST} || '';
my $keeps = $ENV{PERL_STORABLE_KEEP_ID_TEST};

freeze_thaw_test
  (
   name => "object ids between 2G and 4G",
   freeze => \&make_2g_data,
   thaw => \&check_2g_data,
   id => "2g",
   memory => 34,
  );

freeze_thaw_test
  (
   name => "object ids over 4G",
   freeze => \&make_4g_data,
   thaw => \&check_4g_data,
   id => "4g",
   memory => 70,
  );

freeze_thaw_test
  (
   name => "hook object ids over 4G",
   freeze => \&make_hook_data,
   thaw => \&check_hook_data,
   id => "hook4g",
   memory => 70,
  );

# not really an id test, but the infrastructure here makes tests
# easier
freeze_thaw_test
  (
   name => "network store large PV",
   freeze => \&make_net_large_pv,
   thaw => \&check_net_large_pv,
   id => "netlargepv",
   memory => 8,
  );

freeze_thaw_test
    (
     name => "hook store with 2g data",
     freeze => \&make_2g_hook_data,
     thaw => \&check_2g_hook_data,
     id => "hook2gdata",
     memory => 4,
    );

freeze_thaw_test
    (
     name => "hook store with 4g data",
     freeze => \&make_4g_hook_data,
     thaw => \&check_4g_hook_data,
     id => "hook4gdata",
     memory => 8,
    );

sub freeze_thaw_test {
    my %opts = @_;

    my $freeze = $opts{freeze}
      or die "Missing freeze";
    my $thaw = $opts{thaw}
      or die "Missing thaw";
    my $id = $opts{id}
      or die "Missing id";
    my $name = $opts{name}
      or die "Missing name";
    my $memory = $opts{memory}
      or die "Missing memory";
    my $todo_thaw = $opts{todo_thaw} || "";

  SKIP:
    {
	# IPC::Run would be handy here

	$ENV{PERL_TEST_MEMORY} >= $memory
	  or skip "Not enough memory to test $name", 2;
	$skips =~ /\b\Q$id\E\b/
	  and skip "You requested test $name ($id) be skipped", 2;
        defined $keeps && $keeps !~ /\b\Q$id\E\b/
            and skip "You didn't request test $name ($id)", 2;
	my $stored;
	if (defined(my $pid = open(my $fh, "-|"))) {
	    unless ($pid) {
		# child
		open my $cfh, "|-", "gzip"
		  or die "Cannot pipe to gzip: $!";
		binmode $cfh;
		$freeze->($cfh);
		exit;
	    }
	    # parent
	    $stored = do { local $/; <$fh> };
	    close $fh;
	}
	else {
	    skip "$name: Cannot fork for freeze", 2;
	}
	ok($stored, "$name: we got output data")
	  or skip "$name: skipping thaw test", 1;

	my ($tfh, $tname) = tempfile();

	#my $tname = "$id.store.gz";
	#open my $tfh, ">", $tname or die;
	#binmode $tfh;

	print $tfh $stored;
	close $tfh;
    
	if (defined(my $pid = open(my $fh, "-|"))) {
	    unless ($pid) {
		# child
		open my $bfh, "-|", "gunzip <$tname"
		  or die "Cannot pipe from gunzip: $!";
		binmode $bfh;
		$thaw->($bfh);
		exit;
	    }
	    my $out = do { local $/; <$fh> };
	    chomp $out;
	    local $TODO = $todo_thaw;
	    is($out, "OK", "$name: check result");
	}
	else {
	    skip "$name: Cannot fork for thaw", 1;
	}
    }
}


sub make_2g_data {
  my ($fh) = @_;
  my @x;
  my $y = 1;
  my $z = 2;
  my $g2 = 0x80000000;
  $x[0] = \$y;
  $x[$g2] = \$y;
  $x[$g2+1] = \$z;
  $x[$g2+2] = \$z;
  store_fd(\@x, $fh);
}

sub check_2g_data {
  my ($fh) = @_;
  my $x = retrieve_fd($fh);
  my $g2 = 0x80000000;
  $x->[0] == $x->[$g2]
    or die "First entry mismatch";
  $x->[$g2+1] == $x->[$g2+2]
    or die "2G+ entry mismatch";
  print "OK";
}

sub make_4g_data {
  my ($fh) = @_;
  my @x;
  my $y = 1;
  my $z = 2;
  my $g4 = 2*0x80000000;
  $x[0] = \$y;
  $x[$g4] = \$y;
  $x[$g4+1] = \$z;
  $x[$g4+2] = \$z;
  store_fd(\@x, $fh);
}

sub check_4g_data {
  my ($fh) = @_;
  my $x = retrieve_fd($fh);
  my $g4 = 2*0x80000000;
  $x->[0] == $x->[$g4]
    or die "First entry mismatch";
  $x->[$g4+1] == $x->[$g4+2]
    or die "4G+ entry mismatch";
  ${$x->[$g4+1]} == 2
    or die "Incorrect value in 4G+ entry";
  print "OK";
}

sub make_hook_data {
    my ($fh) = @_;
    my @x;
    my $y = HookLargeIds->new(101, { name => "one" });
    my $z = HookLargeIds->new(201, { name => "two" });
    my $g4 = 2*0x8000_0000;
    $x[0] = $y;
    $x[$g4] = $y;
    $x[$g4+1] = $z;
    $x[$g4+2] = $z;
    store_fd(\@x, $fh);
}

sub check_hook_data {
    my ($fh) = @_;
    my $x = retrieve_fd($fh);
    my $g4 = 2*0x8000_0000;
    my $y = $x->[$g4+1];
    $y = $x->[$g4+1];
    $y->id == 201
      or die "Incorrect id in 4G+ object";
    ref($y->data) eq 'HASH'
      or die "data isn't a ref";
    $y->data->{name} eq "two"
      or die "data name not 'one'";
    print "OK";
}

sub make_net_large_pv {
    my ($fh) = @_;
    my $x = "x"; # avoid constant folding making a 4G scalar
    my $g4 = 2*0x80000000;
    my $y = $x x ($g4 + 5);
    nstore_fd(\$y, $fh);
}

sub check_net_large_pv {
    my ($fh) = @_;
    my $x = retrieve_fd($fh);
    my $g4 = 2*0x80000000;
    ref $x && ref($x) eq "SCALAR"
      or die "Not a scalar ref ", ref $x;

    length($$x) == $g4+5
      or die "Incorect length";
    print "OK";
}

sub make_2g_hook_data {
    my ($fh) = @_;

    my $g2 = 0x80000000;
    my $x = HookLargeData->new($g2);
    store_fd($x, $fh);
}

sub check_2g_hook_data {
    my ($fh) = @_;
    my $x = retrieve_fd($fh);
    my $g2 = 0x80000000;
    $x->size == $g2
        or die "Size incorrect ", $x->size;
    print "OK";
}

sub make_4g_hook_data {
    my ($fh) = @_;

    my $g2 = 0x80000000;
    my $g4 = 2 * $g2;
    my $x = HookLargeData->new($g4+1);
    store_fd($x, $fh);
}

sub check_4g_hook_data {
    my ($fh) = @_;
    my $x = retrieve_fd($fh);
    my $g2 = 0x80000000;
    my $g4 = 2 * $g2;
    $x->size == $g4+1
        or die "Size incorrect ", $x->size;
    print "OK";
}

sub find_exe {
    my ($exe) = @_;

    $exe .= $Config{_exe};
    my @path = split /\Q$Config{path_sep}/, $ENV{PATH};
    for my $dir (@path) {
        my $abs = File::Spec->catfile($dir, $exe);
        -x $abs
            and return $abs;
    }
}

package HookLargeIds;

sub new {
    my $class = shift;
    my ($id, $data) = @_;
    return bless { id => $id, data => $data }, $class;
}

sub STORABLE_freeze {
    #print STDERR "freeze called\n";
    #Devel::Peek::Dump($_[0]);

    return $_[0]->id, $_[0]->data;
}

sub STORABLE_thaw {
    my ($self, $cloning, $ser, $data) = @_;

    #Devel::Peek::Dump(\@_);
    #print STDERR "thaw called\n";
    #Devel::Peek::Dump($self);
    $self->{id} = $ser+0;
    $self->{data} = $data;
}

sub id {
    $_[0]{id};
}

sub data {
    $_[0]{data};
}

package HookLargeData;

sub new {
    my ($class, $size) = @_;

    return bless { size => $size }, $class;
}

sub STORABLE_freeze {
    return "x" x $_[0]{size};
}

sub STORABLE_thaw {
    my ($self, $cloning, $ser) = @_;

    $self->{size} = length $ser;
}

sub size {
    $_[0]{size};
}
