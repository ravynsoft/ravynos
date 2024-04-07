BEGIN {
    chdir 't' if -d 't/lib';
    @INC = '../lib' if -d 'lib';
    require Config; import Config;
    if (-d 'lib' and $Config{'extensions'} !~ /\bOS2(::|\/)REXX\b/) {
	print "1..0\n";
	exit 0;
    }
}

# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl test.pl'

######################### We start with some black magic to print on failure.

# Change 1..1 below to 1..last_test_to_print .
# (It may become useful if the test is moved to ./t subdirectory.)

BEGIN { $| = 1; print "1..21\n"; }
END {print "not ok 1\n" unless $loaded;}
use OS2::ExtAttr;
$loaded = 1;
print "ok 1\n";

######################### End of black magic.

# Insert your test code below (better if it prints "ok 13"
# (correspondingly "not ok 13") depending on the success of chunk 13
# of the test code):

unlink 't.out' if -f 't.out';
system 'cmd', '/c', 'echo OK > t.out';

{
  my %a;
  tie %a, 'OS2::ExtAttr', 't.out';
  print "ok 2\n";
  
  keys %a == 0 ? print "ok 3\n" : print "not ok 3\n";
# Standard Extended Attributes (SEAs) have a dot (.) as a prefix.
# This identifies the extended attribute as a SEA. The leading dot is reserved,
# so applications should not define extended attributes that start with a dot.
# Also, extended attributes 
# that start with the characters $, @, &, or + are reserved for system use.
  $a{'X--Y'} = '---';		# '++', -++', '!++', 'X++Y' fail on JFS
  print "ok 4\n";
  $a{'AAA'} = 'xyz';		# Name is going to be uppercased???
  print "ok 5\n";
}

{
  my %a;
  tie %a, 'OS2::ExtAttr', 't.out';
  print "ok 6\n";
  
  my $c = keys %a;
  $c == 2 ? print "ok 7\n" : print "not ok 7\n# c=$c\n";
  my @b = sort keys %a;
  "@b" eq 'AAA X--Y' ? print "ok 8\n" : print "not ok 8\n# keys=`@b'\n";
  $a{'X--Y'} eq '---' ? print "ok 9\n" : print "not ok 9\n";;
  $a{'AAA'} eq 'xyz' ? print "ok 10\n" : print "not ok 10\n# aaa->`$a{AAA}'\n";
  $c = delete $a{'X--Y'};
  $c eq '---' ? print "ok 11\n" : print "not ok 11\n# deleted->`$c'\n";;
}

print "ok 12\n";

{
  my %a;
  tie %a, 'OS2::ExtAttr', 't.out';
  print "ok 13\n";
  
  keys %a == 1 ? print "ok 14\n" : print "not ok 14\n";
  my @b = sort keys %a;
  "@b" eq 'AAA' ? print "ok 15\n" : print "not ok 15\n";
  $a{'AAA'} eq 'xyz' ? print "ok 16\n" : print "not ok 16\n";;
  ! exists $a{'+'} ? print "ok 17\n" : print "not ok 17\n";;
  ! defined $a{'+'} ? print "ok 18\n" : print "not ok 18\n# ->`$a{'X--Y'}'\n";;
  ! exists $a{'X--Y'} ? print "ok 19\n" : print "not ok 19\n";;
  ! defined $a{'X--Y'} ? print "ok 20\n" : print "not ok 20\n# ->`$a{'X--Y'}'\n";;
}

print "ok 21\n";
unlink 't.out';
 
