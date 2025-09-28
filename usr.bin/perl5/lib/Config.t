#!./perl -w

BEGIN {
    chdir 't' if -d 't';
    @INC = '../lib';
    require "./test.pl";

    plan ('no_plan');

    use_ok('Config');
}

use strict;

# Some (safe?) bets.

cmp_ok(keys %Config, '>', 500, "Config has more than 500 entries");

my ($first) = Config::config_sh() =~ /^(\S+)=/m;
die "Can't find first entry in Config::config_sh()" unless defined $first;
note("First entry is '$first'");

# It happens that the we know what the first key should be. This is somewhat
# cheating, but there was briefly a bug where the key got a bonus newline.
my ($first_each) = each %Config;
is($first_each, $first, "First key from each is correct");
ok(exists($Config{$first_each}), "First key exists");
ok(!exists($Config{"\n$first"}),
   "Check that first key with prepended newline isn't falsely existing");

is($Config{PERL_REVISION}, 5, "PERL_REVISION is 5");

# Check that old config variable names are aliased to their new ones.
my %legacy = (
    PERL_VERSION       => 'PATCHLEVEL',
    PERL_SUBVERSION    => 'SUBVERSION',
    PERL_CONFIG_SH     => 'CONFIG'
);
while( my($new, $old) = each %legacy ) {
    isnt($Config{$new}, undef,       "$new is defined");
    is($Config{$new}, $Config{$old}, "$new is aliased to $old");
}

ok( exists $Config{cc},      "has cc");

ok( exists $Config{ccflags}, "has ccflags");

ok(!exists $Config{python},  "has no python");

ok( exists $Config{d_fork},  "has d_fork");

ok(!exists $Config{d_bork},  "has no d_bork");

{
    # check taint_support and tain_disabled are set up as expected.

    ok( exists $Config{taint_support}, "has taint_support");

    ok( exists $Config{taint_disabled}, "has taint_disabled");

    is( $Config{taint_support}, ($Config{taint_disabled} ? "" : "define"),
        "taint_support = !taint_disabled");

    ok( ($Config{taint_support} eq "" or $Config{taint_support} eq "define"),
        "taint_support is a valid value");

    ok( ( $Config{taint_disabled} eq "" or $Config{taint_disabled} eq "silent" or
        $Config{taint_disabled} eq "define"),
        "taint_disabled is a valid value");

    my @opts = Config::non_bincompat_options();
    my @want_taint_disabled = ("", "define", "silent");
    my @want_taint_support = ("define", "", "");
    my ($silent_no_taint_support) = grep $_ eq "SILENT_NO_TAINT_SUPPORT", @opts;
    my ($no_taint_support) = grep $_ eq "NO_TAINT_SUPPORT", @opts;
    my $no_taint_support_count = 0 + grep /NO_TAINT_SUPPORT/, @opts;
    my $want_count = $silent_no_taint_support ? 2 : $no_taint_support ? 1 : 0;

    is ($no_taint_support_count, $want_count,
        "non_bincompat_options info on taint support is as expected");
    is( $Config{taint_disabled}, $want_taint_disabled[$no_taint_support_count],
        "taint_disabled is aligned with non_bincompat_options() data");
    is( $Config{taint_support}, $want_taint_support[$no_taint_support_count],
        "taint_support is aligned with non_bincompat_options() data");
}

like($Config{ivsize}, qr/^(4|8)$/, "ivsize is 4 or 8 (it is $Config{ivsize})");

# byteorder is virtual, but it has rules.

like($Config{byteorder}, qr/^(1234|4321|12345678|87654321)$/,
     "byteorder is 1234 or 4321 or 12345678 or 87654321 "
     . "(it is $Config{byteorder})");

is(length $Config{byteorder}, $Config{ivsize},
   "byteorder is as long as ivsize (which is $Config{ivsize})");

# ccflags_nolargefiles is virtual, too.

ok(exists $Config{ccflags_nolargefiles}, "has ccflags_nolargefiles");

# Utility functions.

{
    # make sure we can export what we say we can export.
    package Foo;
    my @exports = qw(myconfig config_sh config_vars config_re);
    Config->import(@exports);
    foreach my $func (@exports) {
	::ok( __PACKAGE__->can($func), "$func exported" );
    }
}

like(Config::myconfig(), qr/osname=\Q$Config{osname}\E/,   "myconfig");
like(Config::config_sh(), qr/osname='\Q$Config{osname}\E'/, "config_sh");
like(Config::config_sh(), qr/byteorder='[1-8]+'/,
     "config_sh has a valid byteorder");
foreach my $line (Config::config_re('c.*')) {
  like($line,                  qr/^c.*?=.*$/,                   'config_re' );
}

my $out = tie *STDOUT, 'FakeOut';

Config::config_vars('cc');	# non-regex test of essential cfg-var
my $out1 = $$out;
$out->clear;

Config::config_vars('d_bork');	# non-regex, non-existent cfg-var
my $out2 = $$out;
$out->clear;

Config::config_vars('PERL_API_.*');	# regex, tagged multi-line answer
my $out3 = $$out;
$out->clear;

Config::config_vars('PERL_API_.*:');	# regex, tagged single-line answer
my $out4 = $$out;
$out->clear;

Config::config_vars(':PERL_API_.*:');	# regex, non-tagged single-line answer
my $out5 = $$out;
$out->clear;

Config::config_vars(':PERL_API_.*');	# regex, non-tagged multi-line answer
my $out6 = $$out;
$out->clear;

Config::config_vars('PERL_API_REVISION.*:'); # regex, tagged 
my $out7 = $$out;
$out->clear;

# regex, non-tagged multi-line answer
Config::config_vars(':PERL_API_REVISION.*');
my $out8 = $$out;
$out->clear;

Config::config_vars('PERL_EXPENSIVE_.*:'); # non-matching regex
my $out9 = $$out;
$out->clear;

Config::config_vars('?flags');	# bogus regex, no explicit warning !
my $out10 = $$out;
$out->clear;

undef $out;
untie *STDOUT;

like($out1, qr/^cc='\Q$Config{cc}\E';/, "found config_var cc");
like($out2, qr/^d_bork='UNKNOWN';/, "config_var d_bork is UNKNOWN");

# test for leading, trailing colon effects
# Split in scalar context it deprecated, and will warn.
my @tmp;
is(scalar (@tmp = split(/;\n/, $out3)), 3, "3 lines found");
is(scalar (@tmp = split(/;\n/, $out6)), 3, "3 lines found");

is($out4 =~ /(;\n)/s, '', "trailing colon gives 1-line response: $out4");
is($out5 =~ /(;\n)/s, '', "trailing colon gives 1-line response: $out5");

is(scalar (@tmp = split(/=/, $out3)), 4, "found 'tag='");
is(scalar (@tmp = split(/=/, $out4)), 4, "found 'tag='");

my @api;

my @rev = @Config{qw(PERL_API_REVISION PERL_API_VERSION PERL_API_SUBVERSION)};

note("test tagged responses, multi-line and single-line");
foreach my $api ($out3, $out4) {
    @api = $api =~ /PERL_API_(\w+)=(.*?)(?:;\n|\s)/mg;
    is($api[0], "REVISION", "REVISION tag");
    is($api[4], "VERSION",  "VERSION tag");
    is($api[2], "SUBVERSION", "SUBVERSION tag");
    is($api[1], "'$rev[0]'", "REVISION is $rev[0]");
    is($api[5], "'$rev[1]'", "VERSION is $rev[1]");
    is($api[3], "'$rev[2]'", "SUBVERSION is $rev[2]");
}

note("test non-tagged responses, multi-line and single-line");
foreach my $api ($out5, $out6) {
    @api = split /(?: |;\n)/, $api;
    is($api[0], "'$rev[0]'", "revision is $rev[0]");
    is($api[2], "'$rev[1]'", "version is $rev[1]");
    is($api[1], "'$rev[2]'", "subversion is $rev[2]");
}

# compare to each other, the outputs for trailing, leading colon
$out7 =~ s/ $//;
is("$out7;\n", "PERL_API_REVISION=$out8", "got expected diffs");

like($out9, qr/\bnot\s+found\b/, "$out9 - perl is FREE !");
like($out10, qr/\bnot\s+found\b/, "config_vars with invalid regexp");

# Read-only.

undef $@;
eval { $Config{d_bork} = 'borkbork' };
like($@, qr/Config is read-only/, "no STORE");

ok(!exists $Config{d_bork}, "still no d_bork");

undef $@;
eval { delete $Config{d_fork} };
like($@, qr/Config is read-only/, "no DELETE");

ok( exists $Config{d_fork}, "still d_fork");

undef $@;
eval { %Config = () };
like($@, qr/Config is read-only/, "no CLEAR");

ok( exists $Config{d_fork}, "still d_fork");

{
    package FakeOut;

    sub TIEHANDLE {
	bless(\(my $text), $_[0]);
    }

    sub clear {
	${ $_[0] } = '';
    }

    sub PRINT {
	my $self = shift;
	$$self .= join('', @_);
    }
}

# Signal-related variables
# (this is actually a regression test for Configure.)

is($Config{sig_num_init}  =~ tr/,/,/, $Config{sig_size}, "sig_num_init size");
is($Config{sig_name_init} =~ tr/,/,/, $Config{sig_size}, "sig_name_init size");

# Test the troublesome virtual stuff
my @virtual = qw(byteorder ccflags_nolargefiles ldflags_nolargefiles
		 libs_nolargefiles libswanted_nolargefiles);

# Also test that the first entry in config.sh is found correctly. There was
# special casing code for this

foreach my $pain ($first, @virtual) {
  # No config var is named with anything that is a regexp metachar
  ok(exists $Config{$pain}, "\$config('$pain') exists");

  my @result = $Config{$pain};
  is (scalar @result, 1, "single result for \$config('$pain')");

  @result = Config::config_re($pain);
  is (scalar @result, 1, "single result for config_re('$pain')");
  like ($result[0], qr/^$pain=(['"])\Q$Config{$pain}\E\1$/, # grr '
	"which is the expected result for $pain");
}

# Check that config entries appear correctly in @INC
# TestInit.pm has probably already messed with our @INC
# This little bit of evil is to avoid a @ in the program, in case it confuses
# shell 1 liners. We used to use a perl 1-ism, until that was deprecated, so
# now some octal in an eval.
my ($path, $ver, @orig_inc)
  = split /\n/,
    runperl (nolib=>1,
	     prog=>'print qq{$_\n} foreach $^X, $], eval qq{\100INC}');

die "This perl is $] at $^X; other perl is $ver (at $path) "
  . '- failed to find this perl' unless $] eq $ver;

my %orig_inc;
@orig_inc{@orig_inc} = ();

my $failed;
# This [used to be] the order that directories are pushed onto @INC in perl.c:
foreach my $lib (qw(applibexp archlibexp privlibexp sitearchexp sitelibexp
		     vendorarchexp vendorlibexp)) {
  my $dir = $Config{$lib};
  SKIP: {
    skip "lib $lib not in \@INC on Win32" if $^O eq 'MSWin32';
    skip "lib $lib not in \@INC on os390" if $^O eq 'os390';
    skip "lib $lib not defined" unless defined $dir;
    skip "lib $lib not set" unless length $dir;
    # May be in @INC in either Unix or VMS format on VMS.
    if ($^O eq 'VMS' && !exists($orig_inc{$dir})) {
        $dir = VMS::Filespec::unixify($dir);
        $dir =~ s|/$||;
    }
    # So we expect to find it in @INC

    ok (exists $orig_inc{$dir}, "Expect $lib '$dir' to be in \@INC")
      or $failed++;
  }
}
_diag ('@INC is:', @orig_inc) if $failed;
