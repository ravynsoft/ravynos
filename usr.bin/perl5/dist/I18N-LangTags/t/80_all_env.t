use strict;
use Test::More tests => 17;
BEGIN {use_ok('I18N::LangTags::Detect', 1.01);}

note("Using I18N::LangTags::Detect v$I18N::LangTags::Detect::VERSION");

note("Make sure we can assign to ENV entries");
note("Otherwise we can't run the subsequent tests");
$ENV{'MYORP'}   = 'Zing';          is($ENV{'MYORP'}, 'Zing');
$ENV{'SWUZ'}   = 'KLORTHO HOOBOY'; is($ENV{'SWUZ'}, 'KLORTHO HOOBOY');

delete $ENV{'MYORP'};
delete $ENV{'SWUZ'};

sub printenv {
  print "# ENV:\n";
  foreach my $k (sort keys %ENV) {
    my $p = $ENV{$k};  $p =~ s/\n/\n#/g;
    print "#   [$k] = [$p]\n"; }
  print "# [end of ENV]\n#\n";
}

$ENV{'IGNORE_WIN32_LOCALE'} = 1; # a hack, just for testing's sake.

foreach my $test_var (qw (LANGUAGE LC_ALL LC_MESSAGES LANG)) {
    $ENV{$_} = '' foreach qw(REQUEST_METHOD LANGUAGE LC_ALL LC_MESSAGES LANG);
    $ENV{$test_var} = 'Eu-MT';
    my $what = "I18N::LangTags::Detect::detect() for \$ENV{$test_var} = 'Eu-MT'";
    printenv();
    is(scalar I18N::LangTags::Detect::detect(), "eu-mt",
       "scalar $what");
    is_deeply([I18N::LangTags::Detect::detect()], ["eu-mt"], $what);
}

note("Test HTTP_ACCEPT_LANGUAGE");
$ENV{'REQUEST_METHOD'}       = 'GET';

foreach(['eu-MT', "eu-mt"],
	['x-plorp, zaz, eu-MT, i-klung', 
	 'x-plorp', 'i-plorp', 'zaz', 'eu-mt', 'i-klung', 'x-klung'],
	['x-plorp, zaz, eU-Mt, i-klung',
	 'x-plorp', 'i-plorp', 'zaz', 'eu-mt', 'i-klung', 'x-klung'],
       ) {
    my ($val, @expect) = @$_;
    my $what = "I18N::LangTags::Detect::detect() for \$ENV{HTTP_ACCEPT_LANGUAGE} = '$val'";
    $ENV{'HTTP_ACCEPT_LANGUAGE'} = $val;
    printenv();
    is(scalar I18N::LangTags::Detect::detect(), $expect[0], "scalar $what");
    is_deeply([I18N::LangTags::Detect::detect()], \@expect, $what);
}
