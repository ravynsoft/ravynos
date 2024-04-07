#! /usr/local/perl -w
# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl test.pl'

#########################

use Test::More qw/no_plan/;

# do strict lax tests in a sub to isolate a package to test importing
SKIP: {
    skip 'No extended regexes Perl < 5.006', 174
	if $] < 5.006_000;
    strict_lax_tests();
    is ref($version::LAX), 'Regexp', 'Can see $version::LAX '.$version::LAX ;
    is ref($version::STRICT), 'Regexp', 'Can see $version::STRICT '.$version::STRICT;
    { # https://rt.cpan.org/Ticket/Display.html?id=114712
        my ($v) = ( "snapshot-1.2.3ga-001-432" =~ /($version::LAX)/ );
        is $v, '1.2.3', "Extract just the version: $v";
        ($v) = ( "snapshot-1.2ga-001-432" =~ /($version::LAX)/ );
        is $v, '1.2', "Extract just the version: $v";
        ($v) = ( "snapshot-v1.2.3ga-001-432" =~ /($version::STRICT)/ );
        is $v, 'v1.2.3', "Extract just the version: $v";
    }

    is ref($version::LAX_DECIMAL_VERSION), 'Regexp', 'Can see $version::LAX_DECIMAL_VERSION '.$version::LAX_DECIMAL_VERSION ;
    is ref($version::LAX_DOTTED_DECIMAL_VERSION), 'Regexp', 'Can see $version::LAX_DOTTED_DECIMAL_VERSION '.$version::LAX_DOTTED_DECIMAL_VERSION ;
    is ref($version::STRICT_DECIMAL_VERSION), 'Regexp', 'Can see $version::STRICT_DECIMAL_VERSION '.$version::STRICT_DECIMAL_VERSION;
    is ref($version::STRICT_DOTTED_DECIMAL_VERSION), 'Regexp', 'Can see $version::STRICT_DOTTED_DECIMAL_VERSION '.$version::STRICT_DOTTED_DECIMAL_VERSION;
    { # https://rt.cpan.org/Ticket/Display.html?id=119669
        ($v) = ( "snapshot-1.2.3ga-001-432" =~ /($version::LAX_DOTTED_DECIMAL_VERSION)/ );
        is $v, '1.2.3', "Extract just the version: $v";
        ($v) = ( "snapshot-1.2ga-001-432" =~ /($version::LAX_DECIMAL_VERSION)/ );
        is $v, '1.2', "Extract just the version: $v";
        ($v) = ( "snapshot-v1.2.3ga-001-432" =~ /($version::STRICT_DOTTED_DECIMAL_VERSION)/ );
        is $v, 'v1.2.3', "Extract just the version: $v";
        ($v) = ( "snapshot-1.2ga-001-432" =~ /($version::STRICT_DECIMAL_VERSION)/ );
        is $v, '1.2', "Extract just the version: $v";
      }
}


sub strict_lax_tests {
  package temp12345;
  # copied from perl core test t/op/packagev.t
  # format: STRING STRICT_OK LAX_OK
  my $strict_lax_data = << 'CASE_DATA';
1.00		pass	pass
1.00001		pass	pass
0.123		pass	pass
12.345		pass	pass
42		pass	pass
0		pass	pass
0.0		pass	pass
v1.2.3		pass	pass
v1.2.3.4	pass	pass
v0.1.2		pass	pass
v0.0.0		pass	pass
01		fail	pass
01.0203		fail	pass
v01		fail	pass
v01.02.03	fail	pass
.1		fail	pass
.1.2		fail	pass
1.		fail	pass
1.a		fail	fail
1._		fail	fail
1.02_03		fail	pass
v1.2_3		fail	pass
v1.02_03	fail	pass
v1.2_3_4	fail	fail
v1.2_3.4	fail	fail
1.2_3.4		fail	fail
0_		fail	fail
1_		fail	fail
1_.		fail	fail
1.1_		fail	fail
1.02_03_04	fail	fail
1.2.3		fail	pass
v1.2		fail	pass
v0		fail	pass
v1		fail	pass
v.1.2.3		fail	fail
v		fail	fail
v1.2345.6	fail	pass
undef		fail	pass
1a		fail	fail
1.2a3		fail	fail
bar		fail	fail
_		fail	fail
CASE_DATA

  require version;
  'version'->import( qw/is_strict is_lax/ );
  for my $case ( split qr/\n/, $strict_lax_data ) {
    my ($v, $strict, $lax) = split qr/\t+/, $case;
    main::ok( $strict eq 'pass' ? is_strict($v) : ! is_strict($v), "is_strict($v) [$strict]" );
    main::ok( $strict eq 'pass' ? version::is_strict($v) : ! version::is_strict($v), "version::is_strict($v) [$strict]" );
    main::ok( $lax eq 'pass' ? is_lax($v) : ! is_lax($v), "is_lax($v) [$lax]" );
    main::ok( $lax eq 'pass' ? version::is_lax($v) : ! version::is_lax($v), "version::is_lax($v) [$lax]" );
  }
}
