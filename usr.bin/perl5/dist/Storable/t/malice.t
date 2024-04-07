#!./perl -w
#
#  Copyright 2002, Larry Wall.
#
#  You may redistribute only under the same terms as Perl 5, as specified
#  in the README file that comes with the distribution.
#

# I'm trying to keep this test easily backwards compatible to 5.004, so no
# qr//;

# This test tries to craft malicious data to test out as many different
# error traps in Storable as possible
# It also acts as a test for read_header

sub BEGIN {
    # This lets us distribute Test::More in t/
    unshift @INC, 't';
    unshift @INC, 't/compat' if $] < 5.006002;
    require Config; import Config;
    if ($ENV{PERL_CORE} and $Config{'extensions'} !~ /\bStorable\b/) {
        print "1..0 # Skip: Storable was not built\n";
        exit 0;
    }
}

use strict;

our $byteorder = $Config{byteorder};

our $file_magic_str = 'pst0';
our $other_magic = 7 + length $byteorder;
our $network_magic = 2;
our $major = 2;
our $minor = 12;
our $minor_write = $] >= 5.019 ? 11 : $] > 5.008 ? 9 : $] > 5.005_50 ? 8 : 4;

use Test::More;

# If it's 5.7.3 or later the hash will be stored with flags, which is
# 2 extra bytes. There are 2 * 2 * 2 tests per byte in the body and header
# common to normal and network order serialised objects (hence the 8)
# There are only 2 * 2 tests per byte in the parts of the header not present
# for network order, and 2 tests per byte on the 'pst0' "magic number" only
# present in files, but not in things store()ed to memory
our $fancy = ($] > 5.007 ? 2 : 0);

plan tests => 372 + length ($byteorder) * 4 + $fancy * 8;

use Storable qw (store retrieve freeze thaw nstore nfreeze);
require 'testlib.pl';
our $file;

# The chr 256 is a hack to force the hash to always have the utf8 keys flag
# set on 5.7.3 and later. Otherwise the test fails if run with -Mutf8 because
# only there does the hash has the flag on, and hence only there is it stored
# as a flagged hash, which is 2 bytes longer
my %hash = (perl => 'rules', chr 256, '');
delete $hash{chr 256};

sub test_hash {
  my $clone = shift;
  is (ref $clone, "HASH", "Get hash back");
  is (scalar keys %$clone, 1, "with 1 key");
  is ((keys %$clone)[0], "perl", "which is correct");
  is ($clone->{perl}, "rules", "Got expected value when looking up key in clone");
}

sub test_header {
  my ($header, $isfile, $isnetorder) = @_;
  is (!!$header->{file}, !!$isfile, "is file");
  is ($header->{major}, $major, "major number");
  is ($header->{minor}, $minor_write, "minor number");
  is (!!$header->{netorder}, !!$isnetorder, "is network order");
  if ($isnetorder) {
    # Network order header has no sizes
  } else {
    is ($header->{byteorder}, $byteorder, "byte order");
    is ($header->{intsize}, $Config{intsize}, "int size");
    is ($header->{longsize}, $Config{longsize}, "long size");
 SKIP: {
	skip ("No \$Config{prtsize} on this perl version ($])", 1)
	    unless defined $Config{ptrsize};
	is ($header->{ptrsize}, $Config{ptrsize}, "long size");
    }
    is ($header->{nvsize}, $Config{nvsize} || $Config{doublesize} || 8,
        "nv size"); # 5.00405 doesn't even have doublesize in config.
  }
}

sub test_truncated {
  my ($data, $sub, $magic_len, $what) = @_;
  for my $i (0 .. length ($data) - 1) {
    my $short = substr $data, 0, $i;

    # local $Storable::DEBUGME = 1;
    my $clone = &$sub($short);
    is (defined ($clone), '', "truncated $what to $i should fail");
    if ($i < $magic_len) {
      like ($@, "/^Magic number checking on storable $what failed/",
          "Should croak with magic number warning");
    } else {
      is ($@, "", "Should not set \$\@");
    }
  }
}

sub test_corrupt {
  my ($data, $sub, $what, $name) = @_;

  my $clone = &$sub($data);
  local $Test::Builder::Level = $Test::Builder::Level + 1;
  is (defined ($clone), '', "$name $what should fail");
  like ($@, $what, $name);
}

sub test_things {
  my ($contents, $sub, $what, $isnetwork) = @_;
  my $isfile = $what eq 'file';
  my $file_magic = $isfile ? length $file_magic_str : 0;

  my $header = Storable::read_magic ($contents);
  test_header ($header, $isfile, $isnetwork);

  # Test that if we re-write it, everything still works:
  my $clone = &$sub ($contents);

  is ($@, "", "There should be no error");

  test_hash ($clone);

  # Now lets check the short version:
  test_truncated ($contents, $sub, $file_magic
                  + ($isnetwork ? $network_magic : $other_magic), $what);

  my $copy;
  if ($isfile) {
    $copy = $contents;
    substr ($copy, 0, 4) = 'iron';
    test_corrupt ($copy, $sub, "/^File is not a perl storable/",
                  "magic number");
  }

  $copy = $contents;
  # Needs to be more than 1, as we're already coding a spread of 1 minor version
  # number on writes (2.5, 2.4). May increase to 2 if we figure we can do 2.3
  # on 5.005_03 (No utf8).
  # 4 allows for a small safety margin
  # Which we've now exhausted given that Storable 2.25 is writing 2.8
  # (Joke:
  # Question: What is the value of pi?
  # Mathematician answers "It's pi, isn't it"
  # Physicist answers "3.1, within experimental error"
  # Engineer answers "Well, allowing for a small safety margin,   18"
  # )
  my $minor6 = $header->{minor} + 6;
  substr ($copy, $file_magic + 1, 1) = chr $minor6;
  {
    # Now by default newer minor version numbers are not a pain.
    $clone = &$sub($copy);
    is ($@, "", "by default no error on higher minor");
    test_hash ($clone);

    local $Storable::accept_future_minor = 0;
    test_corrupt ($copy, $sub,
                  "/^Storable binary image v$header->{major}\.$minor6 more recent than I am \\(v$header->{major}\.$minor\\)/",
                  "higher minor");
  }

  $copy = $contents;
  my $major1 = $header->{major} + 1;
  substr ($copy, $file_magic, 1) = chr 2*$major1;
  test_corrupt ($copy, $sub,
                "/^Storable binary image v$major1\.$header->{minor} more recent than I am \\(v$header->{major}\.$minor\\)/",
                "higher major");

  # Continue messing with the previous copy
  my $minor1 = $header->{minor} - 1;
  substr ($copy, $file_magic + 1, 1) = chr $minor1;
  test_corrupt ($copy, $sub,
                "/^Storable binary image v$major1\.$minor1 more recent than I am \\(v$header->{major}\.$minor\\)/",
              "higher major, lower minor");

  my $where;
  if (!$isnetwork) {
    # All these are omitted from the network order header.
    # I'm not sure if it's correct to omit the byte size stuff.
    $copy = $contents;
    substr ($copy, $file_magic + 3, length $header->{byteorder})
      = reverse $header->{byteorder};

    test_corrupt ($copy, $sub, "/^Byte order is not compatible/",
                  "byte order");
    $where = $file_magic + 3 + length $header->{byteorder};
    foreach (['intsize', "Integer"],
             ['longsize', "Long integer"],
             ['ptrsize', "Pointer"],
             ['nvsize', "Double"]) {
      my ($key, $name) = @$_;
      $copy = $contents;
      substr ($copy, $where++, 1) = chr 0;
      test_corrupt ($copy, $sub, "/^$name size is not compatible/",
                    "$name size");
    }
  } else {
    $where = $file_magic + $network_magic;
  }

  # Just the header and a tag 255. As 34 is currently the highest tag, this
  # is "unexpected"
  $copy = substr ($contents, 0, $where) . chr 255;

  test_corrupt ($copy, $sub,
                "/^Corrupted storable $what \\(binary v$header->{major}.$header->{minor}\\)/",
                "bogus tag");

  # Now drop the minor version number
  substr ($copy, $file_magic + 1, 1) = chr $minor1;

  test_corrupt ($copy, $sub,
                "/^Corrupted storable $what \\(binary v$header->{major}.$minor1\\)/",
                "bogus tag, minor less 1");
  # Now increase the minor version number
  substr ($copy, $file_magic + 1, 1) = chr $minor6;

  # local $Storable::DEBUGME = 1;
  # This is the delayed croak
  test_corrupt ($copy, $sub,
                "/^Storable binary image v$header->{major}.$minor6 contains data of type 255. This Storable is v$header->{major}.$minor and can only handle data types up to 35/",
                "bogus tag, minor plus 4");
  # And check again that this croak is not delayed:
  {
    # local $Storable::DEBUGME = 1;
    local $Storable::accept_future_minor = 0;
    test_corrupt ($copy, $sub,
                  "/^Storable binary image v$header->{major}\.$minor6 more recent than I am \\(v$header->{major}\.$minor\\)/",
                  "higher minor");
  }
}

ok (defined store(\%hash, $file), "store() returned defined value");

my $expected = 20 + length ($file_magic_str) + $other_magic + $fancy;
my $length = -s $file;

die "Don't seem to have written file '$file' as I can't get its length: $!"
  unless defined $file;

die "Expected file to be $expected bytes (sizeof long is $Config{longsize}) but it is $length"
  unless $length == $expected;

# Read the contents into memory:
my $contents = slurp ($file);

# Test the original direct from disk
my $clone = retrieve $file;
test_hash ($clone);

# Then test it.
test_things($contents, \&store_and_retrieve, 'file');

# And now try almost everything again with a Storable string
my $stored = freeze \%hash;
test_things($stored, \&freeze_and_thaw, 'string');

# Network order.
unlink $file or die "Can't unlink '$file': $!";

ok (defined nstore(\%hash, $file), "nstore() returned defined value");

$expected = 20 + length ($file_magic_str) + $network_magic + $fancy;
$length = -s $file;

die "Don't seem to have written file '$file' as I can't get its length: $!"
  unless defined $file;

die "Expected file to be $expected bytes (sizeof long is $Config{longsize}) but it is $length"
  unless $length == $expected;

# Read the contents into memory:
$contents = slurp ($file);

# Test the original direct from disk
$clone = retrieve $file;
test_hash ($clone);

# Then test it.
test_things($contents, \&store_and_retrieve, 'file', 1);

# And now try almost everything again with a Storable string
$stored = nfreeze \%hash;
test_things($stored, \&freeze_and_thaw, 'string', 1);

# Test that the bug fixed by #20587 doesn't affect us under some older
# Perl. AMS 20030901
{
    chop(my $a = chr(0xDF).chr(256));
    my %a = (chr(0xDF) => 1);
    $a{$a}++;
    freeze \%a;
    # If we were built with -DDEBUGGING, the assert() should have killed
    # us, which will probably alert the user that something went wrong.
    ok(1);
}

# Unusual in that the empty string is stored with an SX_LSCALAR marker
my $hash = store_and_retrieve("pst0\5\6\3\0\0\0\1\1\0\0\0\0\0\0\0\5empty");
ok(!$@, "no exception");
is(ref($hash), "HASH", "got a hash");
is($hash->{empty}, "", "got empty element");
