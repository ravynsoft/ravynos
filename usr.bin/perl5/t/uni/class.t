BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc(qw(../lib .));
    skip_all_without_unicode_tables();
}

plan tests => 12;

my $str = join "", map { chr utf8::unicode_to_native($_) } 0x20 .. 0x6F;

is(($str =~ /(\p{IsMyUniClass}+)/)[0], '0123456789:;<=>?@ABCDEFGHIJKLMNO',
                                'user-defined class compiled before defined');

sub IsMyUniClass {
  my $return = "";
  for my $i (0x30 .. 0x4F) {
    $return .= sprintf("%04X\n", utf8::unicode_to_native($i));
  }
  return $return;
END
}

sub Other::IsClass {
  my $return = "";
  for my $i (0x40 .. 0x5F) {
    $return .= sprintf("%04X\n", utf8::unicode_to_native($i));
  }
  return $return;
}

sub A::B::Intersection {
  <<END;
+main::IsMyUniClass
&Other::IsClass
END
}

sub test_regexp ($$) {
  # test that given string consists of N-1 chars matching $qr1, and 1
  # char matching $qr2
  my ($str, $blk) = @_;

  # constructing these objects here makes the last test loop go much faster
  my $qr1 = qr/(\p{$blk}+)/;
  if ($str =~ $qr1) {
    is($1, substr($str, 0, -1));		# all except last char
  }
  else {
    fail('first N-1 chars did not match');
  }

  my $qr2 = qr/(\P{$blk}+)/;
  if ($str =~ $qr2) {
    is($1, substr($str, -1));			# only last char
  }
  else {
    fail('last char did not match');
  }
}

use strict;

# make sure it finds built-in class
is(($str =~ /(\p{Letter}+)/)[0], 'ABCDEFGHIJKLMNOPQRSTUVWXYZ');
is(($str =~ /(\p{l}+)/)[0], 'ABCDEFGHIJKLMNOPQRSTUVWXYZ');

# make sure it finds user-defined class
is(($str =~ /(\p{IsMyUniClass}+)/)[0], '0123456789:;<=>?@ABCDEFGHIJKLMNO');

# make sure it finds class in other package
is(($str =~ /(\p{Other::IsClass}+)/)[0], '@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_');

# make sure it finds class in other OTHER package
is(($str =~ /(\p{A::B::Intersection}+)/)[0], '@ABCDEFGHIJKLMNO');

# lib/unicore/lib/Bc/AL.pl.  U+070E is unassigned, currently, but still has
# bidi class AL.  The first one in the sequence that doesn't is 0711, which is
# BC=NSM.
$str = "\x{070D}\x{070E}\x{070F}\x{0710}\x{0711}\x{0712}";
is(($str =~ /(\P{BidiClass: ArabicLetter}+)/)[0], "\x{0711}");
is(($str =~ /(\P{BidiClass: AL}+)/)[0], "\x{0711}");
is(($str =~ /(\P{BC :ArabicLetter}+)/)[0], "\x{0711}");
is(($str =~ /(\P{bc=AL}+)/)[0], "\x{0711}");

# make sure InGreek works
$str = "[\x{038B}\x{038C}\x{038D}]";

is(($str =~ /(\p{InGreek}+)/)[0], "\x{038B}\x{038C}\x{038D}");

{   # [perl #133860], compilation before data for it is available
    package Foo;

    sub make {
        my @lines;
        while( my($c) = splice(@_,0,1) ) {
            push @lines, sprintf("%04X", $c);
        }
        return join "\n", @lines;
    }

    my @characters = ( ord("a") );
    sub IsProperty { make(@characters); };

    main::like('a', qr/\p{IsProperty}/, "foo");
}

# The other tests that are based on looking at the generated files are now
# in t/re/uniprops.t
